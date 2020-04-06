//
// Created by Can Yang on 2020/3/22.
//

#include "mm/fmm/ubodt.hpp"
#include "util/util.hpp"
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;
UBODT::UBODT(int buckets_arg, int multiplier_arg) :
    buckets(buckets_arg), multiplier(multiplier_arg) {
  SPDLOG_TRACE("Intialization UBODT with buckets {} multiplier {}",
               buckets, multiplier);
  hashtable = (Record **) malloc(sizeof(Record *) * buckets);
  for (int i = 0; i < buckets; i++) {
    hashtable[i] = nullptr;
  }
  SPDLOG_TRACE("Intialization UBODT finished");
}

UBODT::~UBODT() {
  /* Clean hashtable */
  SPDLOG_TRACE("Clean UBODT");
  int i;
  for (i = 0; i < buckets; ++i) {
    Record *head = hashtable[i];
    Record *curr;
    while ((curr = head) != nullptr) {
      head = head->next;
      free(curr);
    }
  }
  // Destory hash table pointer
  free(hashtable);
  SPDLOG_TRACE("Clean UBODT finished");
}

Record *UBODT::look_up(NodeIndex source, NodeIndex target) const {
  unsigned int h = cal_bucket_index(source, target);
  Record *r = hashtable[h];
  while (r != nullptr) {
    if (r->source == source && r->target == target) {
      return r;
    } else {
      r = r->next;
    }
  }
  return r;
}

std::vector<EdgeIndex> UBODT::look_sp_path(NodeIndex source,
                                           NodeIndex target) const {
  std::vector<EdgeIndex> edges;
  if (source == target) { return edges; }
  Record *r = look_up(source, target);
  // No transition exist from source to target
  if (r == nullptr) { return edges; }
  while (r->first_n != target) {
    edges.push_back(r->next_e);
    r = look_up(r->first_n, target);
  }
  edges.push_back(r->next_e);
  return edges;
}

C_Path UBODT::construct_complete_path(const TGOpath &path,
                                      const std::vector<Edge> &edges,
                                      std::vector<int> *indices) const {
  C_Path cpath;
  if (!indices->empty()) indices->clear();
  if (path.empty()) return cpath;
  int N = path.size();
  cpath.push_back(path[0]->c->edge->id);
  int current_idx = 0;
  indices->push_back(current_idx);
  for (int i = 0; i < N - 1; ++i) {
    const Candidate *a = path[i]->c;
    const Candidate *b = path[i + 1]->c;
    if ((a->edge->id != b->edge->id) || (a->offset > b->offset)) {
      // segs stores edge index
      auto segs = look_sp_path(a->edge->target, b->edge->source);
      // No transition exist in UBODT
      if (segs.empty() && a->edge->target != b->edge->source) {
        indices->clear();
        return C_Path();
      }
      for (int e:segs) {
        cpath.push_back(edges[e].id);
        ++current_idx;
      }
      cpath.push_back(b->edge->id);
      ++current_idx;
      indices->push_back(current_idx);
    } else {
      indices->push_back(current_idx);
    }
  }
  return cpath;
}

double UBODT::get_delta() const {
  return delta;
}

unsigned int UBODT::cal_bucket_index(NodeIndex source, NodeIndex target) const {
  return (source * multiplier + target) % buckets;
}


void UBODT::insert(Record *r) {
  //int h = (r->source*multiplier+r->target)%buckets ;
  int h = cal_bucket_index(r->source, r->target);
  r->next = hashtable[h];
  hashtable[h] = r;
  if (r->cost > delta) delta = r->cost;
}

long UBODT::estimate_ubodt_rows(const std::string &filename) {
  struct stat stat_buf;
  long rc = stat(filename.c_str(), &stat_buf);
  if (rc == 0) {
    int file_bytes = stat_buf.st_size;
    SPDLOG_TRACE("UBODT file size is {} bytes", file_bytes);
    std::string fn_extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(fn_extension.begin(),
                   fn_extension.end(),
                   fn_extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (fn_extension == "csv" || fn_extension == "txt") {
      int row_size = 36;
      return file_bytes / row_size;
    } else if (fn_extension == "bin" || fn_extension == "binary") {
      Record r;
      // When exporting to a file using boost binary writer,
      // the padding is removed.
      int row_size = 28;
      return file_bytes / row_size;
    }
  }
  return -1;
}

int UBODT::find_prime_number(double value) {
  std::vector<int> prime_numbers = {
      5003, 10039, 20029, 50047, 100669, 200003, 500000,
      1000039, 2000083, 5000101, 10000103, 20000033};
  int N = prime_numbers.size();
  for (int i = 0; i < N; ++i) {
    if (value <= prime_numbers[i]) {
      return prime_numbers[i];
    }
  }
  return prime_numbers[N - 1];
}

std::shared_ptr<UBODT> UBODT::read_ubodt_file(const std::string &filename,
    int multiplier) {
  if (UTIL::check_file_extension(filename,"bin")){
    return read_ubodt_binary(filename,multiplier);
  } else if (UTIL::check_file_extension(filename,"csv,txt")) {
    return read_ubodt_csv(filename,multiplier);
  } else {
    SPDLOG_CRITICAL("File format not support: {}",filename);
    std::exit(EXIT_FAILURE);
    return nullptr;
  }
}


std::shared_ptr<UBODT> UBODT::read_ubodt_csv(const std::string &filename,
                                             int multiplier) {
  SPDLOG_INFO("Reading UBODT file (CSV format) from {}", filename);
  long rows = estimate_ubodt_rows(filename);
  int buckets = find_prime_number(rows / LOAD_FACTOR);
  SPDLOG_TRACE("Estimated buckets {}", buckets);
  int progress_step = 1000000;
  std::shared_ptr<UBODT> table = std::make_shared<UBODT>(buckets, multiplier);
  FILE *stream = fopen(filename.c_str(), "r");
  long NUM_ROWS = 0;
  char line[BUFFER_LINE];
  if (fgets(line, BUFFER_LINE, stream)) {
    SPDLOG_TRACE("Header line skipped.");
  }
  while (fgets(line, BUFFER_LINE, stream)) {
    ++NUM_ROWS;
    Record *r = (Record *) malloc(sizeof(Record));
    /* Parse line into a Record */
    sscanf(
        line, "%d;%d;%d;%d;%d;%lf",
        &r->source,
        &r->target,
        &r->first_n,
        &r->prev_n,
        &r->next_e,
        &r->cost
    );
    r->next = nullptr;
    table->insert(r);
    if (NUM_ROWS % progress_step == 0) {
      SPDLOG_INFO("Read rows {}", NUM_ROWS);
    }
  }
  fclose(stream);
  double lf = NUM_ROWS / (double) buckets;
  SPDLOG_TRACE("Estimated load factor #elements/#tablebuckets {}", lf);
  if (lf > 10) { SPDLOG_WARN("Load factor is too large.") }
  SPDLOG_INFO("Finish reading UBODT with rows {}", NUM_ROWS);
  return table;
}

std::shared_ptr<UBODT> UBODT::read_ubodt_binary(const std::string &filename,
                                                 int multiplier) {
  SPDLOG_INFO("Reading UBODT file (binary format) from {}", filename);
  long rows = estimate_ubodt_rows(filename);
  int progress_step = 1000000;
  SPDLOG_TRACE("Estimated rows is {}", rows);
  int buckets = find_prime_number(rows / LOAD_FACTOR);
  std::shared_ptr<UBODT> table = std::make_shared<UBODT>(buckets, multiplier);
  long NUM_ROWS = 0;
  std::ifstream ifs(filename.c_str());
  // Check byte offset
  std::streampos archiveOffset = ifs.tellg();
  std::streampos streamEnd = ifs.seekg(0, std::ios_base::end).tellg();
  ifs.seekg(archiveOffset);
  boost::archive::binary_iarchive ia(ifs);
  while (ifs.tellg() < streamEnd) {
    ++NUM_ROWS;
    Record *r = (Record *) malloc(sizeof(Record));
    ia >> r->source;
    ia >> r->target;
    ia >> r->first_n;
    ia >> r->prev_n;
    ia >> r->next_e;
    ia >> r->cost;
    r->next = nullptr;
    table->insert(r);
    if (NUM_ROWS % progress_step == 0) {
      SPDLOG_INFO("Read rows {}", NUM_ROWS);
    }
  }
  ifs.close();
  double lf = NUM_ROWS / (double) buckets;
  SPDLOG_TRACE("Estimated load factor #elements/#tablebuckets {}", lf);
  if (lf > 10) {
    SPDLOG_WARN("Load factor is too large.")
  }
  SPDLOG_INFO("Finish reading UBODT with rows {}", NUM_ROWS)
  return table;
}
