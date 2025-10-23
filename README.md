# ArgenTag single-cell read demultiplexing pipelines

Demultiplexing refers to the process of identifying the barcode(s) that each sequencing read from a given single-cell sequencing experiment is tagged with. Reads with common barcodes are assigned to the same cell. Currently, there are two alternatives for demultiplexing single-cell data generated on the ArgenTag platform:

+ **Customer-facing pipeline**. For users who want to maintain full control of their analysis or are otherwise unwilling or unable to disclose sequencing data (e.g. due to regulatory requirements), we provide a simplified standalone version of our software which can be run directly by users. This version of the pipeline is briefly described below, and example commands are provided.

+ **In-house single-cell pipeline**. This is the processing pipeline used internally by the ArgenTag team to process internally- and externally-generated data. Typically, users upload their data to ArgenTag's cloud servers, where it is processed by our team to generate demultiplexed read files along with reports and supplementary files. This pipeline is described on a [separate page](doc/in-house.md).

In either case, the main output of the pipeline is a set of demultiplexed, trimmed reads. These can be fed to a downstream analysis pipeline, including the [Iso-Seq pipeline](#pacbio-data-and-iso-seq-downstream-analysis-pipeline) (recommended for PacBio reads) and the [FLAMES-based downstream analysis pipeline](#ont-data-and-flames-based-downstream-analysis-pipeline) (recommended for ONT reads). Downstream analysis is covered here only briefly, but users are encouraged to see the provided [examples](#Examples-and-downstream-analysis) and the documentation for their tool of choice for further details.

## Customer-facing demultiplexing pipeline

![Customer-facing demultiplexing pipeline](doc/img/customer-facing.png)

For the customer-facing pipeline, the entire pipeline (except for an optional [chimera splitting step](#chimera-splitting)) is consolidated into a single binary to make it more user friendly. The input is a file of basecalled reads in sam or fastq format, while the output is a set of demultiplexed, trimmed reads in one of the [supported output formats](#output-formats).

### Usage
    
    Usage: taggy_demux [OPTION...] input-file
    taggy_demux -- a demultiplexer for ArgenTag reads
    
      -D, --max-edit-d=INT/FLOAT Maximum edit distance to consider for linker
                                 alignment (-1 means no limit). If float and < 1,
                                 intepreted as a relative maximum edit dist. If
                                 float and > 1, rounded down. [3]
      -f, --in-fmt=STRING        Format for input read file. Valid values are
                                 "fastq", "sam". [fastq]
      -F, --out-fmt=STRING       Format for output read file. Valid values are
                                 "flames", "fastq", "scnano", "sam". [flames]
      -h, --keep-header          Preserve SAM header in output. Only meaningful if
                                 --in-fmt=sam and --out-fmt=sam. [FALSE]
      -k, --keep-failed          Keep failed reads and set nb tag to -1. Only
                                 meaningful if --out-fmt=sam. [FALSE]
      -o, --output-dir=DIR       Output directory. [Current directory]
      -O, --orient=STRING        Orientation in which to output reads (one of
                                 "sense", "anti", "preserve" or "invert". [sense]
      -p, --trim-poly=STRING     Trim polyA/T from output sequences. Valid values
                                 are "none", "strict", "normal", "lenient". [none]
      -P, --preserve             Preserve tags in original sam record. Only
                                 meaningful if --in-fmt=sam and --out-fmt=sam.
                                 [FALSE]
      -R, --max-r-bases=INT      Maximum number of bases from read to align (-1
                                 means no limit) [-1]
      -t, --trim-TSO             Trim TSO (if found) from output sequences. [FALSE]
                                
      -T, --num-threads=INT      Use INT parallel threads [1]
      -u, --umi-start=INT        Use INT as UMI start coordinate. [26]
      -U, --umi-end=INT          Use INT as UMI end coordinate. [37]
      -w, --whitelist=FILE       Barcode whitelist file. [Default]
      -?, --help                 Give this help list
          --usage                Give a short usage message
    
    Mandatory or optional arguments to long options are also mandatory or optional
    for any corresponding short options.

### Output formats

#### PacBio-style sam format (`--out-fmt=sam`)
This format follows the [SAM format specification](http://samtools.github.io/hts-specs/SAMv1.pdf) maintained by the SAM/BAM Format Specification Working Group, making use of the optional tags to encode additional information relevant to barcode demultiplexing and single-cell analysis. Consistency with the [PacBio BAM format specification](https://pacbiofileformats.readthedocs.io/en/13.0/BAM.html) is maintained. In particular, the following tags are added:

| Tag	| Data type	| Description				|
| ----- | -------------	| -----------				|
| XA    | Z             | Order of tags names. Set to `"XM-CB"`. |
| gp    | i             | Specifies whether or not the barcode for the given read passes. Set to `1` for passing reads. |
| CB	| Z		| Corrected cell barcode. |
| CR	| Z		| Raw (uncorrected) cell barcode. Set equal to CB. |
| XC	| Z		| Raw cell barcode. Set equal to XC. |
| rc	| i		| Predicted real cell. Set to `1` for *all* reads because `taggy_demux` does *not* perform background RNA filtering (elbow plot analysis). See the section on [Updating of the `rc` tag](#Updating-of-the-rc-tag) for details on how to achieve this.|
| nc	| i		| Number of candidate barcodes. Set to `1`. |
| nb	| i		| Edit distance from the barcode for the read to the barcode to which it was reassigned. Set to `0`. |
| XM    | Z             | Raw (after tag) or corrected (after correct) UMI. |

The above does not preclude the presence of other tags previously added by third-party tools, which will be preserved if the `--preserve` flag is set (only meaningful if `--in-fmt=sam` and `--out-fmt=sam`).

An example SAM entry (line) is shown below:

    molecule/0      4       *       0       255     *       *       0       0       GGCAYTCATG[...]CGATGGCTAG *       CB:Z:AACCAAGGAGGTAGAT   XA:Z:XM-CB      XM:Z:CGCGACTGTTCT       ic:i:1  im:Z:m84112_240530_215351_s2/139986042/ccs/40_4082      is:i:1  it:Z:CGCGACTGTTCTAACCAAGGAGGTAGAT       rc:i:1  RG:Z:e4927d21   zm:i:0

<!--

#### PacBio-style bam format (`--out-fmt=bam`)
This is the binary version of the above [PacBio-style sam format](#pacbio-style-sam-format---out-fmtsam), and should be equivalent to using `--out-fmt=sam` followed by sam-to-bam conversion with a third-party tool.

-->

#### FLAMES-style fastq format (`--out-fmt=flames`, default)
This is like the standard fastq format, except that read headers follow the following structure:

    @XXXX-YYYY-ZZZZ_UUUUUUUUUUUU#READID

* `XXXX`, `YYYY` and `ZZZZ` are the 3 barcodes which identify a specific cell.
* `UUUUUUUUUUUU` is the 12-nt UMI.
* `READID` is the original read ID.

An example could be
 
    @0076-0048-0089_ATACCGGCTACA#VH00444:319:AAFV5MHM5:1:1101:18421:23605

which would correspond to sequencing read VH00444:319:AAFV5MHM5:1:1101:18421:23605, which has been tagged with the barcode triplet (0076, 0048, 0089) and the UMI "ATACCGGCTACA".

#### Fastq format with mapped barcode (`--out-fmt=fastq`)
This uses a standard fastq format, except that read headers follow the following structure:

    @BBBBBBBBBBBBBBBB_UUUUUUUUUUUU#READID

* `BBBBBBBBBBBBBBBB` is a unique 16-nt nucleotide pseudobarcode which identifies a specific cell. This results from a mapping from ArgenTag's barcode triplets to a single 16-nt sequence, which is artificial and will not appear verbatim in the original basecalled sequence.
* `UUUUUUUUUUUU` is the 12-nt UMI.
* `READID` is the original read ID.

An example could be
 
    @ACGTGCAGCAGACGGT_ATACCGGCTACA#VH00444:319:AAFV5MHM5:1:1101:18421:23605

which would correspond to sequencing read VH00444:319:AAFV5MHM5:1:1101:18421:23605, which has been tagged with the cell barcode "ACGTGCAGCAGACGGT" and the UMI "ATACCGGCTACA".

#### scNanoGPS-style fastq format (`--out-fmt=scnano`)
This is like the standard fastq format, except that read headers follow the following structure:

    @READID_UUUUUUUUUUUU

* `READID` is the original read ID.
* `UUUUUUUUUUUU` is the 12-nt UMI.

An example could be
 
    @VH00444:319:AAFV5MHM5:1:1101:18421:23605_ATACCGGCTACA

which would correspond to sequencing read VH00444:319:AAFV5MHM5:1:1101:18421:23605, which has been tagged with the UMI "ATACCGGCTACA".

For this format, the barcode is not included in the content of the fastq file, but is instead provided in the file name (one file per barcode combination/cell).

## Chimera splitting

As an optional step before demultiplexing, reads can be run through the `split.sh` bash script to split common chimeric reads. The script takes a fastq file as input and produces a new fastq file where reads with common chimeras have been split. For split reads, each resulting subread keeps the original read ID, followed by and underscore and the subread number (e.g. if read ID `@VH00444:319:AAFV5MHM5:1:1101:18421:23605` is split into two, this will result in two subreads `@VH00444:319:AAFV5MHM5:1:1101:18421:23605_1` and `@VH00444:319:AAFV5MHM5:1:1101:18421:23605_2`). This feature will be incorporated into the main binary in future releases.

### Usage

The basic command for running the chimera splitting script is the following:

    bin/split.sh -i <path_to_input.fastq> -o <path_to_output.fastq>

These and other options are shown below:

    i: Input fastq file.
    o: Output fastq file.
    s: Process only this many reads.
    m: Filter reads smaller than this value.
    M: Filter reads larger than this value.
    G: Use a custom tmp folder. The default is to create via `mktemp -d`.
    C: Use a custom config file.
    R: Use a custom round setup.
    c: Use a custom prefix.
    t: Use this many threads. The default is the number of processors (as reported by nproc) minus one.
    E: Use this maximum edit distance when searching for chimeras. Higher values mean less stringency (more true positives, but also more false positives). An integer is interpreted as an edit distance. A real value < 1 is interpreted as a relative edit distance. The default is 0.12 (12%).
    d: Run in debug mode.
    p: Preserve temporary files instead of deleting.

Processing multiple input files is currently handled externally:

    for inputfile in <path_to_input_files>/*.fastq; do bin/split.sh -i "$inputfile" -o "$inputfile".split; done

# Examples and downstream analysis

## PacBio data and Iso-Seq downstream analysis pipeline

For data generated on the PacBio platform, we recommend using the SAM input format (`--in-fmt=sam`), as well as the PacBio-compatible SAM format (`--out-fmt=sam`). Conversion from SAM to BAM and viceversa can be handled via samtools or similar (ideally in place, via process substitution), as detailed below. The input file is typically a segmented read file obtained from Skera (`segmented.bam` below), but can also be a regular CCS file.

### Example commands
    #Convert input bam file to sam
	samtools view -h segmented.bam > segmented.sam
    #Run demux binary with 32 threads from sam input
    NUM_THREADS=32
	mkdir -p "$OUT_DIR"
    bin/taggy_demux -T "$NUM_THREADS" -o "$OUT_DIR" --orient sense --in-fmt=sam --out-fmt=sam --preserve --trim-TSO --trim-poly normal --keep-header segmented.sam
    #Convert demultiplexed sam output back to bam
    samtools view -bS "$OUT_DIR"/demux.sam > "$OUT_DIR"/demux.bam
	
### Example commands (with in place bam-to-sam conversion)
    #Convert S-read bam file to sam in place via process substitution, and run demux binary with 32 threads
    NUM_THREADS=32
    bin/taggy_demux -T "$NUM_THREADS" -o "$OUT_DIR" --orient sense --in-fmt=sam --out-fmt=sam --preserve --trim-TSO --trim-poly normal --keep-header <(samtools view -h segmented.bam)
    #Convert demultiplexed sam output back to bam
    samtools view -bS "$OUT_DIR"/demux.sam > "$OUT_DIR"/demux.bam

### Updating of the `rc` tag

The demultiplexed read file generated above (`demux.sam/bam`) has the `rc` tag set to `1` for *all* reads because `taggy_demux` does *not* perform background RNA filtering (elbow plot analysis). If their downstream processing pipeline does not include background RNA filtering, users can leverage Described [here](doc/update_rc.md)

![RC tag update workflow](doc/img/rc-update-workflow.png)

### Iso-Seq downstream analysis pipeline

The output from the previous commands is compatible with the [Iso-Seq pipeline, starting from the deduplication step (Step 6)](https://isoseq.how/umi/cli-workflow.html#step-6---deduplication).

## ONT data and FLAMES-based downstream analysis pipeline

For data generated on the ONT platform, we recommend running the optional [chimera splitting step](#chimera-splitting) and outputting in the FLAMES-compatible fastq format (`--out-fmt=flames`) for direct compatibility with our [FLAMES-based downstream analysis pipeline](#FLAMES-based-downstream-analysis-pipeline).

### Example commands for a single fastq file

    #Split chimeras (optional, see "Chimera splitting" above)
    bin/split.sh -i "$INPUT_FASTQ_FILE" -o "$DECHMIERIZED_FASTQ_FILE"
    #Run with 32 threads
	NUM_THREADS=32
    mkdir -p "$OUT_DIR"
    bin/taggy_demux -T "$NUM_THREADS" --in-fmt=fastq --out-fmt=flames -o "$OUT_DIR" --trim-TSO --trim-poly lenient "$DECHMIERIZED_FASTQ_FILE"

### Example commands for multiple fastq files

    #Split chimeras (optional, see "Chimera splitting" above)
	for inputfile in "$INPUT_DIR"/*.fastq
	do
	    bin/split.sh -i "$inputfile" -o "$inputfile".split
	done
    #Run with 32 threads
	NUM_THREADS=32
    mkdir -p "$OUT_DIR"
    bin/taggy_demux -T "$NUM_THREADS" --in-fmt=fastq --out-fmt=flames -o "$OUT_DIR" --trim-TSO --trim-poly lenient <(cat "$INPUT_DIR"/*.fastq.split)

### FLAMES-based downstream analysis pipeline

The output from the previous commands can then be analyzed with our [FLAMES-based downstream analysis pipeline](https://github.com/argentagsw/at_flames).

![FLAMES-based downstream analysis pipeline overview](doc/img/FLAMES-based.png)
