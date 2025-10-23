# Updating of the `rc` tag (`update_rc.sh`)

The demultiplexed read file generated above (`demux.sam/bam`) has the `rc` tag set to `1` for *all* reads because `taggy_demux` does *not* perform background RNA filtering (elbow plot analysis). If their downstream processing pipeline does not include background RNA filtering, users can follow this short guide to update the `rc` tag based on their desired criterion by leveraging `isoseq bcstats` and the provided [convenience script](../bin/update_rc.sh).

![RC tag update workflow](img/rc-update-workflow.png)

## 1. Setup Environment

Create a new environment using the [provided environment file](../res/AT2PB.conda_env.yml):

    conda env create -f res/AT2PB.conda_env.yml
    conda activate at2pb_isoseq

Verify installation:

    samtools --version # Expect: samtools 1.20+
    python --version # Expect: Python 3.10-3.12

## 2. Prepare Script

Make the script executable:

    chmod +x bin/update_rc.sh

##3. Run Workflow

Set your file paths:

    DEMUX="/path/to/demux.bam"
    DEMUX_SORTED="/path/to/demux.sorted.bam"
    BCSTATS_REPORT="/path/to/bcstats_report.tsv"
    OUTPUT="/path/to/corrected.sorted.bam"

Execute the pipeline:

    # Sort demux BAM by cell barcode
    samtools sort -t CB $DEMUX -o $DEMUX_SORTED
    
    # Generate isoseq cell calling stats (knee method by default)
    isoseq bcstats -o $BCSTATS_REPORT $DEMUX_SORTED
    
    # Apply isoseq cell calling to demux sorted BAM
    bin/update_rc.sh -i $DEMUX_SORTED -b $BCSTATS_REPORT -o $OUTPUT -t 8 -v

## 4. Key Arguments

    -i: Input sorted BAM with CB tags (ArgenTag corrected)
    -b: bcstats TSV file from isoseq bcstats
    -o: Output sorted BAM file with correct CB includes rc:i:1 for cells, rc:i:0 for non-cells
    -t: Number of threads (default: 4)
    -v: Verbose output

## 5. Notes

* Output BAM ready for isoseq groupdedup
* Supports any isoseq bcstats percentile method
* Use conda deactivate when finished
