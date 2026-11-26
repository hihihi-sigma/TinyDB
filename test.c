const char *line = "INSERT 101 25.5";
int id;
double score;
int n = sscanf(line, "INSERT %d %lf", &id, &score);