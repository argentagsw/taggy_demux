#!/bin/bash -e

BASE_DIR=$(dirname "$0")
BIN_DIR="$BASE_DIR"/../bin
AWK_DIR="$BASE_DIR"/awk
CMD_DIR="$BASE_DIR"
BASE_RES_DIR="$BASE_DIR"/../res
NUM_THREADS=$(($(nproc) - 1))

INPUT_PATH_OR_FILE=.

ROUNDS=S

ALIGN_BIN="$BIN_DIR"/align
RES_DIR="$BASE_RES_DIR"/demux/config

MAX_EDIT_DIST="0.12"
AMPLICON_LIST="1"

EXP_CODE=split_exp
ROUND_CONFIG_FILE_PARAM=res/roundS_config.sh

while getopts ":i:o:s:m:M:G:C:R:c:t:E:d5Op" opt; do
  case $opt in
    i) INPUT_PATH_OR_FILE="$OPTARG"
    ;;
    o) OUTPUT_FILE="$OPTARG"
    ;;
    s) SAMPLE_SIZE="$OPTARG" 
    ;;
    m) MIN_LENGTH="$OPTARG"
    ;;
    M) MAX_LENGTH="$OPTARG"
    ;;
    G) TMP_DIR_GLOBAL="$OPTARG"
    ;;
    C) ROUND_CONFIG_FILE_PARAM="$OPTARG"
    ;;
    R) ROUNDS="$OPTARG"
    ;;
    c) EXP_CODE="$OPTARG"
    ;;
    t) NUM_THREADS="$OPTARG"
    ;;
    E) MAX_EDIT_DIST="$OPTARG"
    ;;
    d) DEBUG_MODE="Y"
    ;;
    5) FIVE_PRIME_ANCHOR="Y"
    ;;
    O) USE_OFFSETS="Y"
    ;;
    p) PRESERVE_TMP="Y"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    exit 1
    ;;
  esac

  case $OPTARG in
    -*) echo "Option $opt needs a valid argument"
    exit 1
    ;;
  esac
done

if [ ! -z "$DEBUG_MODE" ]
then
  set -x
fi

if [ -d "$INPUT_PATH_OR_FILE" ]; then
  echo "Input is a directory. Expected a file."
  exit 1
else
  echo "Input is not a directory. Assuming basecalled fastq file."
  BASECALLED_DATA_FILE="$INPUT_PATH_OR_FILE"
fi

if [ -z "$TMP_DIR_GLOBAL" ]
then
  TMP_DIR_GLOBAL=$(mktemp -d)
else
  TMP_DIR_GLOBAL=$(mktemp -d -t "$TMP_DIR_GLOBAL")
fi

echo "Filtering reads ${MIN_LENGTH:+shorter than $MIN_LENGTH }${MAX_LENGTH:+longer than $MAX_LENGTH }and counting them"
if [ -z "$SAMPLE_SIZE" ]
then
  cat "$BASECALLED_DATA_FILE" | awk ${MIN_LENGTH:+-v min=$MIN_LENGTH} ${MAX_LENGTH:+-v max=$MAX_LENGTH} -f "$AWK_DIR"/filterByLengthAndCount.awk > "$TMP_DIR_GLOBAL"/size-selected.fastq 3>"$TMP_DIR_GLOBAL"/"$EXP_CODE".lines
else
  SAMPLE_LINES=$(( $SAMPLE_SIZE * 4 ))
  head -n "$SAMPLE_LINES" "$BASECALLED_DATA_FILE" | awk ${MIN_LENGTH:+-v min=$MIN_LENGTH} ${MAX_LENGTH:+-v max=$MAX_LENGTH} -f "$AWK_DIR"/filterByLengthAndCount.awk > "$TMP_DIR_GLOBAL"/size-selected.fastq 3>"$TMP_DIR_GLOBAL"/"$EXP_CODE".lines
fi

echo "Splitting into $NUM_THREADS threads"
total_reads=$(cat "$TMP_DIR_GLOBAL"/"$EXP_CODE".lines)
lines_per_file=$(( ( $(cat "$TMP_DIR_GLOBAL"/"$EXP_CODE".lines) / NUM_THREADS + 1 ) * 4))
if [ -z "$USE_OFFSETS" ]
then
  split -l "$lines_per_file" --numeric-suffixes=0001 --suffix-length=4 --additional-suffix=".fastq" "$TMP_DIR_GLOBAL"/size-selected.fastq "$TMP_DIR_GLOBAL"/size-selected_
else
  for thread in $(seq 1 $NUM_THREADS)
  do
    counter_name="$TMP_DIR_GLOBAL"/counter_thread_$(printf "%04d\n" $thread)
    rm -f "$counter_name"
    mkfifo "$counter_name"
    rm -f "$TMP_DIR_GLOBAL"/size-selected_$(printf "%04d\n" $thread).fastq
    mkfifo "$TMP_DIR_GLOBAL"/size-selected_$(printf "%04d\n" $thread).fastq
  done
  split -l "$lines_per_file" --numeric-suffixes=0001 --suffix-length=4 "$TMP_DIR_GLOBAL"/size-selected.fastq "$TMP_DIR_GLOBAL"/counter_thread_ &
  last=1
  > "$TMP_DIR_GLOBAL"/"$EXP_CODE".byte-counts-offsets
  for thread in $(seq 1 $NUM_THREADS)
  do
    counter_name="$TMP_DIR_GLOBAL"/counter_thread_$(printf "%04d\n" $thread)
    byte_count[$thread]=$(wc -c < $counter_name)
    let byte_offset[$thread]=last+0
    let last+=byte_count[$thread]
    echo -e "${byte_count[$thread]}\t${byte_offset[$thread]}" >> "$TMP_DIR_GLOBAL"/"$EXP_CODE".byte-counts-offsets
    rm $counter_name
  done
fi

for ROUND_NUM in $ROUNDS
do
  echo "Running round $ROUND_NUM."
  TMP_DIR="$TMP_DIR_GLOBAL"/round"$ROUND_NUM"
  rm -rf "$TMP_DIR"
  mkdir -p "$TMP_DIR"

  if [ -z "$ROUND_CONFIG_FILE_PARAM" ]
  then
    ROUND_CONFIG_FILE="$RES_DIR"/default/default_round"$ROUND_NUM"_config.sh
  else
    ROUND_CONFIG_FILE=$(echo "$ROUND_CONFIG_FILE_PARAM" | sed s/xxxxx/"$ROUND_NUM"/ )
  fi
  
  source "$ROUND_CONFIG_FILE"
  
  NUM_PRIMERS=0
  > "$TMP_DIR"/all_primers_auto.fasta 
  for AMPLICON in $AMPLICON_LIST
  do
    NUM_PRIMERS=$((NUM_PRIMERS + 1))
    if [ -z "$FIVE_PRIME_ANCHOR" ]
    then
      echo ">CS1_EPF_F_$AMPLICON" >> "$TMP_DIR"/all_primers_auto.fasta 
      varname=SEQ_CS1_EPF_F_"$AMPLICON"
      echo "${!varname}" >> "$TMP_DIR"/all_primers_auto.fasta
    else
      echo ">CS1_EPF_F_$AMPLICON" >> "$TMP_DIR"/all_primers_auto.fasta 
      varname=SEQ_BPF_"$AMPLICON"
      echo "${!varname}" >> "$TMP_DIR"/all_primers_auto.fasta
    fi
    varname=BRK_OFFSET_"$AMPLICON"
    BRK_OFFSETS="${BRK_OFFSETS:+$BRK_OFFSETS,}${!varname}"
  done
  
  for thread in $(seq 1 $NUM_THREADS)
  do
    thread_file="$TMP_DIR_GLOBAL"/size-selected_$(printf "%04d\n" $thread).fastq
    if [ ! -z "$USE_OFFSETS" ]
    then
      tail -c +${byte_offset[$thread]} "$TMP_DIR_GLOBAL"/size-selected.fastq | head -c ${byte_count[$thread]} > "$thread_file" &
    fi
    "$ALIGN_BIN" ${MAX_R_BASES:+--max-r-bases="$MAX_R_BASES"} ${MAX_P_BASES:+--max-p-bases="$MAX_P_BASES"} --num-primers="$NUM_PRIMERS" --max-edit-d="$MAX_EDIT_DIST" "$TMP_DIR"/all_primers_auto.fasta "$thread_file" > "$TMP_DIR"/size-selected_$(printf "%04d\n" $thread)_parsed_primer_hits.dat &
  done

  wait

  echo "Break offsets: $BRK_OFFSETS"
  for thread in $(seq 1 $NUM_THREADS)
  do
    thread_file="$TMP_DIR_GLOBAL"/size-selected_$(printf "%04d\n" $thread).fastq
    if [ ! -z "$USE_OFFSETS" ]
    then
      tail -c +${byte_offset[$thread]} "$TMP_DIR_GLOBAL"/size-selected.fastq | head -c ${byte_count[$thread]} > "$thread_file" &
    fi
    awk -v brk_offset=$BRK_OFFSETS -f "$AWK_DIR"/split_fastq.awk "$TMP_DIR"/size-selected_$(printf "%04d\n" $thread)_parsed_primer_hits.dat "$thread_file" > "$TMP_DIR"/size-selected_$(printf "%04d\n" $thread)_split.fastq &
  done

  wait

  cat "$TMP_DIR"/size-selected_*_parsed_primer_hits.dat > "$TMP_DIR"/parsed_primer_hits.dat
  cat "$TMP_DIR"/size-selected_*_split.fastq > "$OUTPUT_FILE"
  if [ -z "$DEBUG_MODE" ] && [ -z "$PRESERVE_TMP" ]
  then
    rm -r "$TMP_DIR"
  fi
done

if [ -z "$DEBUG_MODE" ] && [ -z "$PRESERVE_TMP" ]
then
  rm "$TMP_DIR_GLOBAL"/size-selected_*.fastq
  rm "$TMP_DIR_GLOBAL"/size-selected.fastq
  rm "$TMP_DIR_GLOBAL"/"$EXP_CODE".lines
  rm -r "$TMP_DIR_GLOBAL"
fi

echo "Finished running split.sh on $INPUT_PATH_OR_FILE"

