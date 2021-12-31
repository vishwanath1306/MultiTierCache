#include<stdio.h>
#include<libCacheSim.h>
#include "multitiersimulator.h"

bool write_stat_to_file(char* op_fname, traceStats multi_tier_stats){
    FILE *out_file = fopen(op_fname, "a");

    if(out_file == NULL){
        return false;
    }
    fprintf(out_file, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", multi_tier_stats.total_count, multi_tier_stats.read_count,
     multi_tier_stats.write_count, multi_tier_stats.tier_1_read_hit, multi_tier_stats.tier_2_read_hit, multi_tier_stats.tier_1_read_miss, 
     multi_tier_stats.tier_2_read_miss, multi_tier_stats.tier_1_write_hit, multi_tier_stats.tier_2_write_hit, multi_tier_stats.tier_1_write_miss,
     multi_tier_stats.tier_2_write_miss, multi_tier_stats.tier_1_total_hit, multi_tier_stats.tier_2_total_hit, multi_tier_stats.tier_1_total_miss, 
     multi_tier_stats.tier_2_total_miss, multi_tier_stats.tier_2_not_checked);

    fclose(out_file);

    return true;
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
    
    write_stat_to_file(op_fname, multi_tier_stats);

}


traceStats exclusive_caching(char *filename, uint64_t tier_1_size, uint64_t tier_2_size, uint64_t obj_id, uint64_t op){
    reader_init_param_t init_csv = {
        .delimiter=',',
        .obj_id_field=obj_id,
        .op_field=op,
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
                exclusive_stats.tier_1_total_hit++;
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

    // write_stat_to_file(op_fname, exclusive_stats);
    return exclusive_stats;
}


opTypeStats trace_raw_stats(char *input_filename, uint64_t object_id, uint64_t op_field){

    reader_init_param_t init_csv = {
        .delimiter=',',
        .obj_id_field=object_id,
        .op_field=op_field,
        .has_header=false
    };

    opTypeStats file_stats = {0, 0};

    reader_t *reader_csv = open_trace(input_filename, CSV_TRACE, OBJ_ID_NUM, &init_csv);
    request_t *req = new_request();
    
    while (read_one_req(reader_csv, req) == 0){
        if(req->op == OP_READ){
            file_stats.read_count++;
        }

        if(req->op == OP_WRITE){

            file_stats.write_count++;
        }
    }
    
    return file_stats;
}

void read_file(char* filename){
    reader_init_param_t init_csv = {
        .delimiter=',',
        .obj_id_field=2,
        .op_field=3,
        .has_header=false
    };

    reader_t *reader_csv = open_trace(filename, CSV_TRACE, OBJ_ID_NUM, &init_csv);
    request_t *req = new_request();
    int i = 0;
    while(read_one_req(reader_csv, req) == 0){
        printf("Sno: %d, Obj: %ld, Op: %d\n", i, req->obj_id_int, req->op);
        ++i;
    }

    free_request(req);
    close_reader(reader_csv);
}

// int main(int argc, char* argv[]){

//     if(argc != 6){
//         printf("Usage: ./new.o <file_name> <tier_1_size> <tier_2_size> <output_file> <incl_excl(0,1)>\n");
//     }
//     uint64_t tier_1_size = atoi(argv[2]);
//     uint64_t tier_2_size = atoi(argv[3]);
//     uint64_t incl_excl = atoi(argv[5]);

//     // read_file(argv[1]);
//     opTypeStats values = trace_raw_stats(argv[1], 2, 3);
//     printf("Read Count is: %d\n", values.read_count);
//     printf("Write Count is: %d\n", values.write_count);
//     // if(incl_excl == 0){
//     //     inclusive_caching(argv[1], tier_1_size, tier_2_size, argv[4]);
//     // }else{
//     //     exclusive_caching(argv[1], tier_1_size, tier_2_size, argv[4]);
//     // }
// }