# ArgenTAG single-cell software pipelines


## In-house single-cell pipeline

![In-house single-cell pipeline overview](img/in-house.png)
### bam2fastq

Takes FASTQ Reads from the ONT Dorado basecaller in HAC/SUP modes or the PacBio Kinnex Skera output
For basecallers which produce bam output, conversion to fastq is required. This can be readily achieved with gnutils, samtools and/or dedicated tools.

### darwin.sh

Screens the barcoding architecture of FASTQ reads.
Samples reads and clusters them into "species" based on shared barcode and adapter patterns.
Generates a report of "species" of reads for visual inspection of library artifacts, including chimeric reads.

### split.sh

If chimeric reads are detected, split.sh can be optionally run to split them and generates dechmierized raw read files, suitable for demultiplexing.
The darwin tool can optionally be ran again and a second report generated to check for successfull chimera splitting (dechmierization).

### demux.sh

This is the core demultiplexing tool.
* Uses a one-shot mathematical decoding algorithm to detect and identify BC triplets in individual reads.
* Operates autonomously without requiring complementary short reads.
* Scales efficiently with respect to the number of BC triplets, avoiding exhaustive alignment to external whitelists.
* Generates a matrix of barcode calls with their corresponding confidences (.dat).
* Furth.er details on ArgenTAG barcoding tech are available [here](https://pubmed.ncbi.nlm.nih.gov/27259539/).

### post\_demux.sh

Takes the barcode calls and confidences and applies sanity checks and filtering criteria to filter out dubious barcode calls, untagged molecules, adapters and other unwanted reads. Generates FASTQ files with confident associations of transcript reads to BC triplets in FASTQ headers ready for cell calling

## Customer-facing single-cell pipeline

![Customer-facing single-cell pipeline](img/customer-facing.png)

# Downstream analysis

## FLAMES-based downstream analysis pipeline

![FLAMES-based downstream analysis pipeline overview](img/FLAMES-based.png)

Performs gene and transcript quantification at the cell level
Generates gene and transcript count matrices from minimap2 alignment of FASTQ cell files to a genome reference and its GFF3 annotation file
Produces gene and GFF3 isoform annotation files
This module reuses parts of [FLAMES](https://github.com/mritchielab/FLAMES/) version 1.9.0, date 2023-10-02, for transcript quantification and isoform annotation.


* Input: FASTQ Cell Files
* Output: Gene Count Matrix (CSV), Transcript Count Matrix (CSV), Isoform Annotation Files (GFF3) 

### AT ISe
Description:
Acts as an interface between FlamesCounter outputs and Seurat inputs
Generate a genes x BCs matrix (R object) suitable for Seurat input
Converts original gene IDs to gene names to ensure Seurat compatibility

Input: Gene Count Matrix (CSV)
Output: Gene Count Matrix (R) 

### Seurat

Third-party tool, included for reference/completeness. Reference version is v.5.0.3.

Input: Gene Count Matrix (R matrix) 
Output: Seurat Final Results 
Description:
Implements major components for QC, analysis, and exploration of single-cell RNA-seq data at the gene level
* Performs quality control on the gene count matrix, removing low-quality cells and genes.
* Applies standard LogNormalization and identifies highly variable genes.
* Determines the dimensionality of the filtered gene count matrix using PCA.
* Generates UMAP clusters of cells and produces gene marker files for these clusters.

For detailed methodologies, please refer to the respective documentation for [Seurat](https://satijalab.org/seurat/articles/get_started_v5_new).

### SQANTI3 5.2.1

Third-party tool, included for reference/completeness. Reference version is v.5.2.1.
Input: Raw Isoform GFF3 annotation file  
Output: Polished Isoform Annotation file (GFF3), SQANTI3 Final Results
Description:
* Implements quality control, filtering, and characterization of long read-defined transcriptomes at the bulk level.
* Generates a raw isoform report with classification categories and QC metrics.
* Produces a curated isoform report by applying automatic isoform filtering and rescue rules.
* Rules for isoform curation are sequencing platform-specific; ONT rules are more stringent than PacBio ones.

For detailed methodologies, please refer to the respective documentation for [SQANTI3](https://github.com/ConesaLab/SQANTI3/wiki/Introduction-to-SQANTI3).

### AT SCISO (under development)
Input: Polished GFF3 Isoform Annotation Files, Transcript Count Matrix (CSV)
Output: Final SCISO Results
Description:
* Cleans the given Transcript Count Matrix with Polished Isoform Annotation Files.
* Generates an isoform-based UMAP clustering of cells.

