// this file will be included from the main project
//
namespace rocfft {

void initialize();

void finalize();

void* create_batch_plan(int direction, int rank, int* dims, int dist, int nfft, bool auto_alloc);

void destroy_plan(void* plan);

void execute_plan(void* plan, void* buffer);

size_t get_work_size(void* handler);

}
