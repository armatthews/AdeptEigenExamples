# If adept.h isn't found try "module load adept-1.0"
g++ main.cpp -Wall -ladept -lboost_serialization -g -O3 -std=c++11 -L/usr/local/lib/
