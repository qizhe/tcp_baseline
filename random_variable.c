#include "random_variable.h"
#include <math.h>

void init_empirical_random_variable(struct empirical_random_variable* r, const char* filename, bool smooth) {
  r->smooth = smooth;
  r->minCDF_ = 0;
  r->maxCDF_ = 1;
  r->maxEntry_ = 65536;
  r->table_ = (CDFentry*) malloc(sizeof(struct CDFentry) * r->maxEntry_);
  loadCDF(r, filename);
}


void init_exp_random_variable(struct exp_random_variable* r, double avg) {
  r->min_ = 0;
  r->max_ = 1.0;
  r->avg_ = avg;

}
int loadCDF(struct empirical_random_variable* r, const char* filename) {
  //assert(false);
  char line[1000];
  FILE * fptr;
  r->numEntry_ = 0;
  double prev_cd = 0;
  int prev_sz = 1;
  double w_sum = 0;

  if((fptr = fopen(filename, "r")) == NULL) {
    exit(EXIT_FAILURE);
  }
  while(fgets(line, 1000, fptr) != NULL) {
    double f;
    sscanf(line, "%lf %lf %lf", &r->table_[r->numEntry_].val_, &f, &r->table_[r->numEntry_].cdf_);

    double freq = r->table_[r->numEntry_].cdf_ - prev_cd;
    double flow_sz = r->smooth?(r->table_[r->numEntry_].val_ + prev_sz)/2.0 : r->table_[r->numEntry_].val_;
    w_sum += freq * flow_sz;
    prev_cd = r->table_[r->numEntry_].cdf_;
    prev_sz = r->table_[r->numEntry_].val_;
    r->numEntry_ ++;
  }
  r->mean_flow_size = w_sum * 1460.0;
  //std::cout << "Mean flow size derived from CDF file:" << this->mean_flow_size << " smooth = " << this->smooth << "\n";
  //std::cout << "Number of lines in text file: " << numEntry_ << "\n";
  fclose(fptr);
  return r->numEntry_;
}

int lookup(struct empirical_random_variable* r, double u) {
  // always return an index whose value is >= u
  int lo, hi, mid;
  if (u <= r->table_[0].cdf_){
    return 0;
  }
  lo=1, hi= r->numEntry_-1;
  for (; lo < hi; ) {
    mid = (lo + hi) / 2;
    if (u > r->table_[mid].cdf_)
      lo = mid + 1;
    else
      hi = mid;
    }
  return lo;
}

double value_emp(struct empirical_random_variable* r) {
  if (r->numEntry_ <= 0)
    return 0;
  int rand_max = 1000000;
  double u = (double)(rand() % rand_max) / rand_max;
  int mid = lookup(r, u);
  if (mid && u < r->table_[mid].cdf_)
  return interpolate(u, r->table_[mid-1].cdf_, r->table_[mid-1].val_,
         r->table_[mid].cdf_, r->table_[mid].val_);
  return r->table_[mid].val_;
}

double value_exp(struct exp_random_variable* r) {
  int rand_max = 1000000;
  double unif0_1 = (double)(rand() % rand_max) / rand_max;
  double value = r->min_ + (r->max_ - r->min_) * unif0_1;
  return -1.0 * r->avg_ * log(value);
}

double interpolate(double x, double x1, double y1,
    double x2, double y2) {
  double value = y1 + (x - x1) * (y2 - y1) / (x2 - x1);
  return value;
}