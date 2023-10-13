#ifndef ENSR_H__
#define ENSR_H__

struct ensr_config {
  _Bool verbose;
};

int ensr_main(struct ensr_config *cfg);

#endif 
