#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <fstream>
#include <random>
#include <algorithm>

constexpr size_t VEC_SIZE = 64;
constexpr float THRESHOLD_SPLIT = 0.85f;
constexpr int NOISE_THRESHOLD = 3; // Branches that have passed less than 3 signals are considered noise

// Auxiliary similarity function
float cosineSimilarity(const float* A, const float* B) {
    float dot_product = 0.0f, norm_A = 0.0f, norm_B = 0.0f;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        dot_product += A[i] * B[i];
        norm_A += A[i] * A[i];
        norm_B += B[i] * B[i];
    }
    if (norm_A == 0.0f || norm_B == 0.0f) return 0.0f;
    return dot_product / (std::sqrt(norm_A) * std::sqrt(norm_B));
}

class MicroModule {
public:
    std::string name;
    float template_vector[VEC_SIZE];
    std::vector<std::shared_ptr<MicroModule>> children;
    int pass_count = 0; // USAGE COUNTER (For Sleep)

    MicroModule(std::string n, const float* tmpl) : name(n) {
        if (tmpl) std::copy(tmpl, tmpl + VEC_SIZE, template_vector);
        else std::fill(template_vector, template_vector + VEC_SIZE, 0.0f);
    }

    void processVector(const float* input) {
        pass_count++; // We increase the counter on each pass
        
        if (children.empty()) return; 

        std::shared_ptr<MicroModule> best_child = nullptr;
        float max_sim = -1.0f;

        for (const auto& child : children) {
            float sim = cosineSimilarity(input, child->template_vector);
            if (sim > max_sim) {
                max_sim = sim;
                best_child = child;
            }
        }

        if (max_sim >= THRESHOLD_SPLIT) {
            best_child->processVector(input);
        } else {
            // mitosis
            std::string new_name = name + "_Branch_" + std::to_string(children.size() + 1);
            auto new_child = std::make_shared<MicroModule>(new_name, input);
            children.push_back(new_child);
            new_child->processVector(input);
        }
    }

    // --- SLEEP ALGORITHM: APOPTOSIS ---
    // Recursively removes branches that turn out to be random noise
    void sleepRefactoringApoptosis() {
        // We remove children whose pass_count is less than the threshold.
        children.erase(
            std::remove_if(children.begin(), children.end(),
                [](const std::shared_ptr<MicroModule>& child) {
                    return child->pass_count < NOISE_THRESHOLD;
                }),
            children.end()
        );

        // Reset counters for the next day and recursive call
        pass_count = 0; 
        for (auto& child : children) {
            child->sleepRefactoringApoptosis();
        }
    }

    // --- SLEEP ALGORITHM: MERGE OF SYNONYMS ---
    void sleepRefactoringMerge() {
        if (children.empty()) return;

        const float MERGE_THRESHOLD = 0.92f; // Split threshold (must be higher than THRESHOLD_SPLIT)

        // We compare each branch with each other
        for (size_t i = 0; i < children.size(); ++i) {
            for (size_t j = i + 1; j < children.size(); ) {
                
                float sim = cosineSimilarity(children[i]->template_vector, children[j]->template_vector);
                
                // If two branches are too similar, they are synonyms, merge them!
                if (sim >= MERGE_THRESHOLD) {
                    std::cout << "      [MERGE] Combining branches: " << children[i]->name 
                              << " + " << children[j]->name << "\n";

                    int total_passes = children[i]->pass_count + children[j]->pass_count;
                    
                    if (total_passes > 0) {
                        for (size_t k = 0; k < VEC_SIZE; ++k) {
                            // Weighted average using the center of mass formula
                            float weighted_sum = (children[i]->template_vector[k] * children[i]->pass_count) + 
                                                 (children[j]->template_vector[k] * children[j]->pass_count);
                            children[i]->template_vector[k] = weighted_sum / total_passes;
                        }
                    }

                    // We pass on the accumulated experience to the surviving branch
                    children[i]->pass_count = total_passes; 

                    // L2 Normalize new vector
                    float sum_sq = 0.0f;
                    for (size_t k = 0; k < VEC_SIZE; ++k) sum_sq += children[i]->template_vector[k] * children[i]->template_vector[k];
                    if (sum_sq > 0.0f) {
                        float norm_factor = std::sqrt(sum_sq);
                        for (size_t k = 0; k < VEC_SIZE; ++k) children[i]->template_vector[k] /= norm_factor;
                    }

                    // Remove the absorbed branch j
                    children.erase(children.begin() + j);
                } else {
                    ++j;
                }
            }
        }

        // Recursively call for all surviving children
        for (auto& child : children) {
            child->sleepRefactoringMerge();
        }
    }

    // Counting nodes for memory measurement
    int countTotalNodes() const {
        int count = 1;
        for (const auto& child : children) {
            count += child->countTotalNodes();
        }
        return count;
    }
};

// Random Noise Addition Function (Real World Simulation)
void addNoiseAndNormalize(float* vec, std::mt19937& gen) {
    std::uniform_real_distribution<float> dist(-0.3f, 0.3f);
    float sum_sq = 0.0f;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        vec[i] += dist(gen);
        if (vec[i] < 0.0f) vec[i] = 0.0f;
        sum_sq += vec[i] * vec[i];
    }
    if (sum_sq > 0.0f) {
        float norm = std::sqrt(sum_sq);
        for (size_t i = 0; i < VEC_SIZE; ++i) vec[i] /= norm;
    }
}

// Simulation of a systematic conveyor defect (for example, camera skew)
void addSystematicDefect(float* vec) {
    // Hard-change the first 15 pixels of the vector
    for(int k = 0; k < 15; ++k) {
        vec[k] = 1.0f; 
    }
    
    // Let's normalize
    float sum_sq = 0.0f;
    for (size_t i = 0; i < VEC_SIZE; ++i) sum_sq += vec[i] * vec[i];
    if (sum_sq > 0.0f) {
        float norm = std::sqrt(sum_sq);
        for (size_t i = 0; i < VEC_SIZE; ++i) vec[i] /= norm;
    }
}

int main() {
    std::cout << "Starting Edge Continuous Learning Stress Test...\n";
    
    // Preparing a CSV file
    std::ofstream csv_file("memory_log.csv");
    csv_file << "Iteration,TotalNodes,MemoryBytes,Phase\n";

    // Initialization of the root and base vector
    auto root = std::make_shared<MicroModule>("Root", nullptr);
    float base_signal[VEC_SIZE];
    std::fill(base_signal, base_signal + VEC_SIZE, 0.1f); // Conventional "Unit"
    
    // --- Fix: Give the tree its first basic branch (DNA) ---
    auto base_branch = std::make_shared<MicroModule>("Base_DNA", base_signal);
    root->children.push_back(base_branch);
    // -------------------------------------------------------------
    
    // Randomness generator
    std::random_device rd;
    std::mt19937 gen(rd());

    int iteration = 0;
    const int BYTES_PER_NODE = 256; // Approximate size of a node with a vector in C++

    // PHASE 1: "Morning" - Ideal Data (1000 cycles)
    for (int i = 0; i < 1000; ++i, ++iteration) {
        root->processVector(base_signal);
        csv_file << iteration << "," << root->countTotalNodes() << "," 
                 << root->countTotalNodes() * BYTES_PER_NODE << ",Morning_Clean\n";
    }

    // PHASE 2: "Day" - Noise + 20% Systematic Defects (4000 cycles)
    for (int i = 0; i < 4000; ++i, ++iteration) {
        float signal[VEC_SIZE];
        std::copy(base_signal, base_signal + VEC_SIZE, signal);
        
        if (i % 5 == 0) {
            // Every 5th iteration there is a SYSTEMIC defect
            addSystematicDefect(signal);
        } else {
            // In other cases - random white noise (dust on the sensor)
            addNoiseAndNormalize(signal, gen);
        }
        
        root->processVector(signal);
        
        if (i % 10 == 0) {
            csv_file << iteration << "," << root->countTotalNodes() << "," 
                     << root->countTotalNodes() * BYTES_PER_NODE << ",Day_Noisy\n";
        }
    }

    // PHASE 3: "Sleep" - Refactoring (1 cycle)
    std::cout << "Node count BEFORE Sleep: " << root->countTotalNodes() << "\n";
    root->sleepRefactoringApoptosis(); // It will clean up the noise, but will leave a system defect!
    root->sleepRefactoringMerge();
    std::cout << "Node count AFTER Sleep: " << root->countTotalNodes() << "\n";
    
    csv_file << iteration << "," << root->countTotalNodes() << "," 
             << root->countTotalNodes() * BYTES_PER_NODE << ",Night_Sleep\n";
    iteration++;

    // PHASE 4: "New Morning" - The conveyor has been cleared of dust, leaving only a systemic imbalance.
    for (int i = 0; i < 2000; ++i, ++iteration) {
        float signal[VEC_SIZE];
        std::copy(base_signal, base_signal + VEC_SIZE, signal);
        
        if (i % 5 == 0) {
            // The defect is still here
            addSystematicDefect(signal); 
        } 
        // There is no more white noise (simulating the normal operation of the learned system)
        
        root->processVector(signal);
        
        if (i % 10 == 0) {
            csv_file << iteration << "," << root->countTotalNodes() << "," 
                     << root->countTotalNodes() * BYTES_PER_NODE << ",Next_Morning\n";
        }
    }

    csv_file.close();
    std::cout << "Test complete. Results saved to 'memory_log.csv'.\n";
    return 0;
}
