#include "ResultData.h"

using namespace std;


void json(ostream& out, Results& results, AlleleParser* parser) {
    out << "{";
    for (Results::iterator r = results.begin(); r != results.end(); ++r) {
        ResultData& sample = r->second;
        if (r != results.begin()) out << ",";
        out << "\"" << sample.name << "\":{"
            << "\"coverage\":" << sample.observations.size() << ","
            << "\"genotypes\":[";
        for (map<Genotype*, long double>::iterator g = sample.marginals.begin(); 
                g != sample.marginals.end(); ++g) {
            if (g != sample.marginals.begin()) cout << ",";
            out << "[\"" << *(g->first) << "\"," << exp(g->second) << "]";
        }
        out << "]";
        if (parser->parameters.outputAlleles)
            out << ",\"alleles\":" << json(sample.observations);
        out << "}";

    }
    out << "}";
}

// current date string in YYYYMMDD format
string dateStr(void) {

    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y%m%d", timeinfo);

    return string(buffer);

}

void vcfHeader(ostream& out,
        string referenceName,
        vector<string>& samples) {

    out << "##fileformat=VCFv4.0" << endl
        << "##fileDate=" << dateStr() << endl
        << "##source=alleleBayes" << endl
        << "##reference=" << referenceName << endl
        << "##phasing=none" << endl
        << "##INFO=<ID=NS,Number=1,Type=Integer,Description=\"number of samples with data\">" << endl
        << "##INFO=<ID=DP,Number=1,Type=Integer,Description=\"total read depth at the locus\">" << endl
        << "##INFO=<ID=AC,Number=1,Type=Integer,Description=\"total number of alternate alleles in called genotypes\">" << endl
        << "##INFO=<ID=AF,Number=1,Type=Integer,Description=\"total number of alleles in called genotypes\">" << endl
        << "##INFO=<ID=ESF,Number=1,Type=Float,Description=\"Ewens' sampling formula probability for the called genotype combination\">" << endl
        << "##FORMAT=GT,1,String,\"Genotype\"" << endl
        << "##FORMAT=GQ,1,Integer,\"Genotype Quality\"" << endl
        << "##FORMAT=DP,1,Integer,\"Read Depth\"" << endl
        << "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER\tINFO\tFORMAT";
    for (vector<string>::iterator s = samples.begin(); s != samples.end(); ++s) {
        out << "\t" << *s;
    }
    out << endl;

}

void vcf(ostream& out,
        long double comboProb,
        long double alleleSamplingProb,
        string alternateBase,
        vector<string>& samples,
        list<Allele*> observedAlleles,
        Results& results,
        AlleleParser* parser) {

    string refbase = parser->currentReferenceBase();
    // positional information
    // CHROM  POS  ID  REF  ALT  QUAL  FILTER  INFO  FORMAT
    out << parser->currentTarget->seq << "\t"
        << parser->currentPosition + 1 << "\t"
        << "." << "\t"
        << refbase << "\t"
        << alternateBase << "\t"
        << float2phred(1 - comboProb) << "\t"
        << "." << "\t" // filter, no filter applied
        << "NS=" << results.size() << ";" << "DP=" << observedAlleles.size() << ";" << "ESF=" << alleleSamplingProb << "\t" // positional information
        << "GT:GQ:DP";

    // samples
    for (vector<string>::iterator sampleName = samples.begin(); sampleName != samples.end(); ++sampleName) {
        Results::iterator s = results.find(*sampleName);
        if (s != results.end()) {
            ResultData& sample = s->second;
            pair<Genotype*, long double> bestGenotypeAndProb = sample.bestMarginalGenotype();
            Genotype& bestGenotype = *bestGenotypeAndProb.first;
            out << "\t"
                << bestGenotype.relativeGenotype(refbase)
               << ":" << float2phred(1 - exp(bestGenotypeAndProb.second))
                << ":" << sample.observations.size();
        } else {
            out << "\t.";
        }
    }
}

pair<Genotype*, long double> ResultData::bestMarginalGenotype(void) {
    map<Genotype*, long double>::iterator g = marginals.begin();
    pair<Genotype*, long double> best = make_pair(g->first, g->second); ++g;
    for ( ; g != marginals.end() ; ++g) {
        if (g->second > best.second) {
            best = make_pair(g->first, g->second);
        }
    }
    return best;
}

// TODO vcf output

