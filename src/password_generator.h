#ifndef PASSWORD_GENERATOR_H
#define PASSWORD_GENERATOR_H

#define MAX_PASSWORD_LENGTH 256
#define MAX_PATTERNS 1000
#define MAX_MEMORY_SIZE 10000

typedef struct {
    char *base_words[20];
    int base_words_count;
    char *common_suffixes[10];
    int suffixes_count;
    char *common_prefixes[5];
    int prefixes_count;
    char *transformations[5];
    int transformations_count;
    char *special_chars[10];
    int special_chars_count;
} PasswordPatterns;

typedef struct {
    PasswordPatterns patterns;
    char **password_memory;
    int memory_size;
    int memory_capacity;
    double learning_rate;
} AIPasswordGenerator;

typedef struct {
    char password[MAX_PASSWORD_LENGTH];
    int attempts;
    double duration;
    int success;
    char error[256];
} AttackResult;

typedef struct {
    double length_weights[3];
    double with_special_char;
    double with_numbers;
    double mixed_case;
} NeuralWeights;

AIPasswordGenerator* ai_generator_init(void);
void ai_generator_free(AIPasswordGenerator *gen);
char* generate_context_aware_password(AIPasswordGenerator *gen, const char *username, int attempt_number);
char* generate_advanced_ai_password(AIPasswordGenerator *gen, const char *username, char **previous_attempts, int prev_count);
char* leet_speak(const char *text);
char* generate_neural_password(NeuralWeights *weights, AIPasswordGenerator *gen, const char *username);
NeuralWeights predict_next_password_type(char **failed_attempts, int fail_count);
double calculate_ai_delay(int attempt, double base_delay);

#endif
