import simulator

data = simulator.read_write_stats("w97_1000.csv", 2, 3)
return_val = simulator.exclusive_cache("w97_1000.csv", 10, 20, 2, 3, "w97_100_result.res")
print(return_val)