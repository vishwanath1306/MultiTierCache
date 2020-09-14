#include<stdio.h>
#include<libCacheSim.h>


void compute_hits(char* filename, uint64_t tier_1_size, uint64_t tier_2_size){
    
    reader_init_param_t init_csv = {
        .delimiter=',',
        .obj_id_field=1,
        .op_field=2,
        .has_header=false
    };

    reader_t *reader_csv = open_trace(filename, CSV_TRACE, OBJ_ID_NUM, &init_csv);
    request_t *req = new_request();

    common_cache_params_t tier_1_params = {.cache_size=tier_1_size};
    common_cache_params_t tier_2_params = {.cache_size=tier_2_size};

    cache_t *tier_1_cache = create_cache("LRU", tier_1_params, NULL);
    cache_t *tier_2_cache = create_cache("LRU", tier_2_params, NULL);

    while(read_one_req(reader_csv, req) == 0){
        cache_ck_res_e tier_1_check = tier_1_cache->check(tier_1_cache, req, true);
        cache_ck_res_e tier_2_check = tier_2_cache->check(tier_2_cache, req, true); 
        
        printf("Request is: %ld, Request type: %d\n", req->obj_id_int, req->op);
        if((tier_1_check == cache_ck_miss) && (tier_2_check == cache_ck_miss)){
            printf("Miss in both layers\n");
            tier_1_cache->get(tier_1_cache, req);
            tier_2_cache->get(tier_2_cache, req);   
        }

        if((tier_1_check == cache_ck_miss) && (tier_2_check != cache_ck_miss)){
            printf("Object missed in tier 1\n");
            tier_1_cache->get(tier_1_cache, req);
            tier_2_cache->get(tier_2_cache, req);
        }

        if((tier_1_check != cache_ck_miss) && (tier_2_check != cache_ck_miss)){
            printf("Object hit in both tiers\n");
            tier_1_cache->get(tier_1_cache, req);
            tier_2_cache->get(tier_2_cache, req);
        }
    }
}

int main(int argc, char* argv[]){

    uint64_t tier_1_size = atoi(argv[2]);
    uint64_t tier_2_size = atoi(argv[3]);

    compute_hits(argv[1], tier_1_size, tier_2_size);


}