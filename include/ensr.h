#ifndef ensrH__
#define ensrH__

struct ensr_config {
  _Bool verbose;
};

int ensr_main(struct ensr_config *cfg);

#endif 
