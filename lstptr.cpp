#include <vector>
#include "meta.hpp"
using namespace std;

Lstptr::Lstptr(vector<mDatanp> np, Indexer& ind){

    string namebase = np.filename;
    long start_pos = np.start_pos;
    long end_pos = np.end_pos;
    long meta_doc_start = np.meta_doc_start;
    long meta_freq_start = np.meta_freq_start;
    long posting_start = np.posting_start;

    last_id = ind.decompress_np(namebase, start_pos, meta_doc_start);
    doc_block_size = ind.decompress_np(namebase, meta_doc_start, meta_freq_start);
    freq_block_size = ind.decompress_np(namebase, meta_freq_start, posting_start);

    unsigned int docID = 0;
    dsizeit = doc_block_size.begin();
    lidit = last_id.begin();
}

void Lstptr::operator++(){

}
