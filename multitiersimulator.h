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


typedef struct{
    int read_count;
    int write_count;
}opTypeStats;