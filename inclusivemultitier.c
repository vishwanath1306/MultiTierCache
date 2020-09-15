#include<stdio.h>
#include<libCacheSim.h>

typedef struct{
    int tier_1_read_hit;
    int tier_2_read_hit;

    int tier_1_write_hit;
    int tier_2_write_hit;
    
    int tier_1_read_miss;
    int tier_2_read_miss;

    int tier_1_write_miss;
    int tier_2_write_miss;
    
    int tier_1_total_hit;
    int tier_1_total_miss;
    
    int tier_2_total_hit;
    int tier_2_total_miss;

    int read_count;
    int write_count;
    int total_count;
    
}traceStats;


void compute_hits(char* filename, uint64_t tier_1_size, uint64_t tier_2_size, char* op_fname){
    
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

    traceStats multi_tier_stats = {0,0,0,0,0,0,0,0,0};

    while(read_one_req(reader_csv, req) == 0){
        cache_ck_res_e tier_1_check = tier_1_cache->check(tier_1_cache, req, true);
        cache_ck_res_e tier_2_check = tier_2_cache->check(tier_2_cache, req, true); 
        
        if((tier_1_check == cache_ck_miss) && (tier_2_check == cache_ck_miss)){
            // Condition where the object is not found in both layers.             
            tier_1_cache->get(tier_1_cache, req);
            tier_2_cache->get(tier_2_cache, req);   
            if(req->op == OP_READ){
                multi_tier_stats.tier_1_read_miss++;
                multi_tier_stats.tier_2_read_miss++;
                multi_tier_stats.read_count++;
            }else{
                multi_tier_stats.tier_1_write_miss++;
                multi_tier_stats.tier_2_write_miss++;
                multi_tier_stats.write_count++;
            }
            multi_tier_stats.total_count++;
            multi_tier_stats.tier_1_total_miss++;
            multi_tier_stats.tier_2_total_miss++;
        }

        if((tier_1_check == cache_ck_miss) && (tier_2_check != cache_ck_miss)){
            // Condition where the object is found in layer 2, but not layer 1
            tier_1_cache->get(tier_1_cache, req);
            tier_2_cache->get(tier_2_cache, req);
            if(req->op == OP_READ){
                multi_tier_stats.tier_1_read_miss++;
                multi_tier_stats.tier_2_read_hit++;
                multi_tier_stats.read_count++;
            }else{
                multi_tier_stats.tier_1_write_miss++;
                multi_tier_stats.tier_2_write_hit++;
                multi_tier_stats.write_count++;
            }

            multi_tier_stats.total_count++;
            multi_tier_stats.tier_1_total_miss++;
            multi_tier_stats.tier_2_total_hit++;
        }

        if((tier_1_check != cache_ck_miss) && (tier_2_check != cache_ck_miss)){
            // Condition where the object is found in both layers. 
            tier_1_cache->get(tier_1_cache, req);
            tier_2_cache->get(tier_2_cache, req);

            if(req->op == OP_READ){
                multi_tier_stats.tier_1_read_hit++;
                multi_tier_stats.tier_2_read_hit++;
                multi_tier_stats.read_count++;
            }else{
                multi_tier_stats.tier_1_write_hit++;
                multi_tier_stats.tier_2_write_hit++;
                multi_tier_stats.write_count++;
            }

            multi_tier_stats.total_count++;
            multi_tier_stats.tier_1_total_hit++;
            multi_tier_stats.tier_2_total_hit++;
        }
    }

    tier_1_cache->cache_free(tier_1_cache);
    tier_2_cache->cache_free(tier_2_cache);

    free_request(req);
    close_reader(reader_csv);

    FILE *out_file = fopen(op_fname, "a");
    fprintf(out_file, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", multi_tier_stats.total_count, multi_tier_stats.read_count,
     multi_tier_stats.write_count, multi_tier_stats.tier_1_read_hit, multi_tier_stats.tier_2_read_hit, multi_tier_stats.tier_1_read_miss, 
     multi_tier_stats.tier_2_read_miss, multi_tier_stats.tier_1_write_hit, multi_tier_stats.tier_2_write_hit, multi_tier_stats.tier_1_write_miss,
     multi_tier_stats.tier_2_write_miss, multi_tier_stats.tier_1_total_hit, multi_tier_stats.tier_2_total_hit, multi_tier_stats.tier_1_total_miss, 
     multi_tier_stats.tier_2_total_miss);

     fclose(out_file);

}


void compute_hits_2(char *filename, uint64_t tier_1_size, uint64_t tier_2_size, char* op_fname){
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
        print_request(req);

        if(unlikely(tier_1_cache->occupied_size < tier_1_cache->cache_size)){
            if(tier_1_check == cache_ck_hit){
                printf("Tier 1 cache hit. Size lower.\n");
                tier_1_cache->get(tier_1_cache, req);
            }else{
                printf("Tier 1 cache miss. Size lower.\n");
                tier_1_cache->get(tier_1_cache, req);
            } 
        }else{
            if(tier_1_check == cache_ck_hit){
                printf("Tier 1 cache hit. Size higher.\n");
                tier_1_cache->get(tier_1_cache, req);
            }else if((tier_1_check == cache_ck_miss) && (tier_2_check == cache_ck_hit)){
                printf("Tier 1 miss. Tier 2 hit. Size lower.\n");
                cache_obj_t evicted_object;
                tier_1_cache->evict(tier_1_cache, req, &evicted_object);
                tier_1_cache->get(tier_1_cache, req);
                req->obj_id_int = evicted_object.obj_id_int;
                req->op = OP_WRITE;
                tier_2_cache->get(tier_2_cache, req);
            }else if((tier_1_check == cache_ck_miss) && (tier_1_check == cache_ck_miss)){
                printf("Miss in both tiers\n");
                cache_obj_t evicted_object;
                tier_1_cache->evict(tier_1_cache, req, &evicted_object);
                tier_1_cache->get(tier_1_cache, req);
                req->obj_id_int = evicted_object.obj_id_int;
                req->op = OP_WRITE;
                tier_2_cache->get(tier_2_cache, req);
            }    
        }
        
    }

    tier_1_cache->cache_free(tier_1_cache);
    tier_2_cache->cache_free(tier_2_cache);

    free_request(req);
    close_reader(reader_csv);
}
int main(int argc, char* argv[]){

    uint64_t tier_1_size = atoi(argv[2]);
    uint64_t tier_2_size = atoi(argv[3]);

    // compute_hits(argv[1], tier_1_size, tier_2_size, argv[4]);
    compute_hits_2(argv[1], tier_1_size, tier_2_size, argv[4]);
}