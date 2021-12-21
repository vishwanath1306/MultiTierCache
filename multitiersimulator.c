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
    
    int tier_2_not_checked; // Only for exclusive caches, if there's a hit in layer 1. 
}traceStats;


void write_stat_to_file(char* op_fname, traceStats multi_tier_stats){
    FILE *out_file = fopen(op_fname, "a");

    fprintf(out_file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", multi_tier_stats.total_count, multi_tier_stats.read_count,
     multi_tier_stats.write_count, multi_tier_stats.tier_1_read_hit, multi_tier_stats.tier_2_read_hit, multi_tier_stats.tier_1_read_miss, 
     multi_tier_stats.tier_2_read_miss, multi_tier_stats.tier_1_write_hit, multi_tier_stats.tier_2_write_hit, multi_tier_stats.tier_1_write_miss,
     multi_tier_stats.tier_2_write_miss, multi_tier_stats.tier_1_total_hit, multi_tier_stats.tier_2_total_hit, multi_tier_stats.tier_1_total_miss, 
     multi_tier_stats.tier_2_total_miss, multi_tier_stats.tier_2_not_checked);

    fclose(out_file);
}

void inclusive_caching(char* filename, uint64_t tier_1_size, uint64_t tier_2_size, char* op_fname){
    
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

    traceStats multi_tier_stats = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1};

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
            // tier_2_cache->get(tier_2_cache, req);

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

    // FILE *out_file = fopen(op_fname, "a");
    // fprintf(out_file, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", multi_tier_stats.total_count, multi_tier_stats.read_count,
    //  multi_tier_stats.write_count, multi_tier_stats.tier_1_read_hit, multi_tier_stats.tier_2_read_hit, multi_tier_stats.tier_1_read_miss, 
    //  multi_tier_stats.tier_2_read_miss, multi_tier_stats.tier_1_write_hit, multi_tier_stats.tier_2_write_hit, multi_tier_stats.tier_1_write_miss,
    //  multi_tier_stats.tier_2_write_miss, multi_tier_stats.tier_1_total_hit, multi_tier_stats.tier_2_total_hit, multi_tier_stats.tier_1_total_miss, 
    //  multi_tier_stats.tier_2_total_miss);

    //  fclose(out_file);
    write_stat_to_file(op_fname, multi_tier_stats);

}


void exclusive_caching(char *filename, uint64_t tier_1_size, uint64_t tier_2_size, char* op_fname){
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

    traceStats exclusive_stats = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1};
    exclusive_stats.tier_2_not_checked++;
    while(read_one_req(reader_csv, req) == 0){
        cache_ck_res_e tier_1_check = tier_1_cache->check(tier_1_cache, req, true);
        cache_ck_res_e tier_2_check = tier_2_cache->check(tier_2_cache, req, true);
        exclusive_stats.total_count++;
        // print_request(req);

        if(unlikely(tier_1_cache->occupied_size < tier_1_cache->cache_size)){
            exclusive_stats.tier_2_not_checked++;
            if(tier_1_check == cache_ck_hit){
                // printf("Tier 1 cache hit. Size lower.\n");
                tier_1_cache->get(tier_1_cache, req);
                if(req->op == OP_WRITE){
                    exclusive_stats.write_count++;
                    exclusive_stats.tier_1_write_hit++;
                }else{
                    exclusive_stats.read_count++;
                    exclusive_stats.tier_1_read_hit++;
                }
                exclusive_stats.tier_1_total_hit;
            }else{
                // printf("Tier 1 cache miss. Size lower.\n");
                tier_1_cache->get(tier_1_cache, req);
                if(req->op == OP_WRITE){
                    exclusive_stats.write_count++;
                    exclusive_stats.tier_1_write_miss++;
                }else{
                    exclusive_stats.read_count++;
                    exclusive_stats.tier_1_read_miss++;
                }
            } 
        }else{
            if(tier_1_check == cache_ck_hit){
                exclusive_stats.tier_2_not_checked++;
                // printf("Tier 1 cache hit. Size higher.\n");
                tier_1_cache->get(tier_1_cache, req);
                if(req->op == OP_WRITE){
                    exclusive_stats.write_count++;
                    exclusive_stats.tier_1_write_hit++;
                }else{
                    exclusive_stats.read_count++;
                    exclusive_stats.tier_1_read_hit++;
                }
                exclusive_stats.tier_1_total_hit++;
            }else if((tier_1_check == cache_ck_miss) && (tier_2_check == cache_ck_hit)){
                // printf("Tier 1 miss. Tier 2 hit. Size lower.\n");
                if(req->op == OP_READ){
                    exclusive_stats.read_count++;
                    exclusive_stats.tier_1_read_miss++;
                    exclusive_stats.tier_2_read_hit++;
                    exclusive_stats.tier_1_total_miss++;
                    exclusive_stats.tier_2_total_hit++;
                }else{
                    exclusive_stats.write_count++;
                    exclusive_stats.tier_1_write_miss++;
                    exclusive_stats.tier_2_write_hit++;
                    exclusive_stats.tier_1_total_miss++;
                    exclusive_stats.tier_2_total_hit++;                
                }
                cache_obj_t evicted_object;
                tier_1_cache->evict(tier_1_cache, req, &evicted_object);
                tier_1_cache->get(tier_1_cache, req);
                req->obj_id_int = evicted_object.obj_id_int;
                req->op = OP_WRITE;
                tier_2_cache->get(tier_2_cache, req);
            }else if((tier_1_check == cache_ck_miss) && (tier_1_check == cache_ck_miss)){
                // printf("Tier 1 miss. Tier 2 hit. Size lower.\n");
                if(req->op == OP_READ){
                    exclusive_stats.read_count++;
                    exclusive_stats.tier_1_read_miss++;
                    exclusive_stats.tier_2_read_miss++;
                    exclusive_stats.tier_1_total_miss++;
                    exclusive_stats.tier_2_total_miss++;
                }else{
                    exclusive_stats.write_count++;
                    exclusive_stats.tier_1_write_miss++;
                    exclusive_stats.tier_2_write_miss++;
                    exclusive_stats.tier_1_total_miss++;
                    exclusive_stats.tier_2_total_miss++;                
                }
                // printf("Miss in both tiers\n");
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

    write_stat_to_file(op_fname, exclusive_stats);
}
int main(int argc, char* argv[]){

    if(argc != 6){
        printf("Usage: ./new.o <file_name> <tier_1_size> <tier_2_size> <output_file> <incl_excl(0,1)>\n");
    }
    uint64_t tier_1_size = atoi(argv[2]);
    uint64_t tier_2_size = atoi(argv[3]);
    uint64_t incl_excl = atoi(argv[5]);
    if(incl_excl == 0){
        inclusive_caching(argv[1], tier_1_size, tier_2_size, argv[4]);
    }else{
        exclusive_caching(argv[1], tier_1_size, tier_2_size, argv[4]);
    }
}