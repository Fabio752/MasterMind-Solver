
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <math.h>

void set_random_seed();
int randn(int n);
void poss_sol(std::vector<std::vector<int> >& soln, int length, int num, std::vector<int>& one_solution);
void first_move(std::vector<int>& attempt, int length, int num);
void feedback_generator(std::vector<int> sequence, std::vector<int> attempt, int& black_hits, int& white_hits, int length);
int b_h_feedback(std::vector<int>& sequence, std::vector<int>& attempt, int length);
int w_h_feedback(std::vector<int>& sequence, std::vector<int>& attempt, int length);

struct feedback{
    int black_hits;
    int white_hits;
};
/// this is the struct definition for the code maker
struct mm_code_maker{
    
    void init(int i_length, int i_num){
        length = i_length;
        num = i_num;
    }
    
    void generate_sequence(){
        for(int i = 0; i < length; i++)
            sequence.push_back(randn(num));
    }
    
    void give_feedback(const std::vector<int>& attempt, int& black_hits, int& white_hits){
        black_hits = 0;
        white_hits = 0;
        std::vector<int> presences(num, 0);
        std::vector<bool> flag(length, false);
        
        for(int i = 0; i < length; i++)
            presences[sequence[i]]++;
        
        for(int i = 0; i < length; i++)
            if(attempt[i] == sequence[i]){
                black_hits++;
                presences[attempt[i]]--;
                flag[i] = true;
            }
        for(int i = 0; i < length; i++)
            if(!flag[i] && presences[attempt[i]] > 0){
                white_hits++;
                presences[attempt[i]]--;
            }
    }
    
    int length;
    int num;
    std::vector<int> sequence;
};


struct mm_solver{

    void init(int i_length, int i_num){
        length = i_length;
        num = i_num;
        num_tested = 0;
        turn = 0;
        scalable_approach = what_approach();
        binary_search_index_start = 0;
        binary_search_index_end = length / 2;
        init_solution_vector();
        index_highest_feedback = 0;
        mask = num;
    }
    
    void create_attempt(std::vector<int>& attempt){
        if(!scalable_approach){
            if(turn == 0)
                first_move(attempt, length, num);
            else{
                //initialise the attempt to all -1s
                for (int i = 0; i < length; i++)
                    attempt.push_back(-1);
            
                //check the attempt that gave more feedback informations save it in the variable closest_attempt
                check_highest_feedback();
                std::vector<int> closest_attempt;
                for(int i = 0; i < length; i++)
                    closest_attempt.push_back(attempts_store[index_highest_feedback][i]);
                
                int max_white_hits = feedback_store[index_highest_feedback].white_hits;
                int max_black_hits = feedback_store[index_highest_feedback].black_hits;
                
                bool valid_attempt = false;
                while(!valid_attempt){
                    
                    //reset the attempt to all -1s
                    for (int i = 0; i < length; i++)
                        attempt[i] = -1;
                    
                    generate_next_attempt(max_black_hits, max_white_hits, closest_attempt, attempt);
                    valid_attempt = check_attempt_validity(attempt);
                    for(int i = 0; i < length; i++)
                        if(attempt[i] == -1)
                            valid_attempt = false;
                }
            }
        }
        
        else
            scalable_create_attempt(attempt);
    
        turn++;
    }
    
    void learn(std::vector<int>& attempt, int black_hits, int white_hits){
        if(!scalable_approach){
            //pushback last attempt made in a vector that stores all the attempts.
            attempts_store.push_back(attempt);
            
            //pushback last attempt's feedback in a vector that stores all the attempts' feedbacks.
            feedback attempt_feedback;
            attempt_feedback.black_hits = black_hits;
            attempt_feedback.white_hits = white_hits;
            feedback_store.push_back(attempt_feedback);
        }
        else
            scalable_learn(attempt, black_hits, white_hits);
    }
    
    
    void  generate_next_attempt(int black_hits, int white_hits, std::vector<int> closest_attempt, std::vector<int>& attempt){

        std::vector<bool> flag_number_used(length, false);
        std::vector<bool> positions_tried(length, false);
        bool still_chances = true;

        //firstly let's position the white hits
        while(white_hits > 0){
            int pos_to_overwrite = randn(length);
            
            while(attempt[pos_to_overwrite] != -1)
                pos_to_overwrite = randn(length);
            
            int pos_considered_white_peg = randn(length);
            
            //while the random number is a position already used generate another one
            while(((flag_number_used[pos_considered_white_peg] == true || closest_attempt[pos_to_overwrite] == closest_attempt[pos_considered_white_peg]) && still_chances) || pos_to_overwrite == pos_considered_white_peg){
                pos_considered_white_peg = randn(length);
                positions_tried[pos_considered_white_peg] = true;
                
                //if all the positions are flagged means that something went wrong so we quit the while loop in order not to be stuck in an infinite loop and the program will start over the generation of the next attempt.
                still_chances = false;
                for(int i = 0; i < length; i++)
                    if(positions_tried[i] == false)
                        still_chances = true;
            }
            
            flag_number_used[pos_considered_white_peg] = true;
            attempt[pos_to_overwrite] = closest_attempt[pos_considered_white_peg];
            white_hits--;
        }
        
        //we now check if the white hits generated give us the possibility to place black hits as needed
        int possible_black_hits = 0;
        for(int i = 0; i < length; i++)
            if(attempt[i] == -1 && flag_number_used[i] == false)
                possible_black_hits++;
        
        if(black_hits <= possible_black_hits && still_chances){
            //if yes we proceed to place the black hits
            while(black_hits > 0){
                int random_pos = randn(length);
                while(attempt[random_pos] != -1 || flag_number_used[random_pos] == true)
                    random_pos = randn(length);
                
                attempt[random_pos] = closest_attempt[random_pos];
                flag_number_used[random_pos] = true;
                black_hits--;
            }
            //finally in all the position of the attempt where we neither placed a black nor a white hit we generate a random number that would not result neither black nor white hit.
            for(int i = 0; i < length; i++)
                if(attempt[i] == -1){
                    int rand_num;
                    bool valid_offset = false;
                    
                    while(!valid_offset){
                        rand_num = randn(num);
                        valid_offset = true;
                        
                        for (int j = 0; j < length; j++)
                            if (flag_number_used[j] == false && closest_attempt[j] == rand_num)
                                valid_offset = false;
                        
                        if(rand_num == closest_attempt[i])
                            valid_offset = false;
                    }
                    attempt[i] = rand_num;
                }
        }
    }

    
    bool check_attempt_validity(std::vector<int> attempt){
        for(int i = 0; i < attempts_store.size(); i++){
            int b_h = 0;
            int w_h = 0;
            give_feedback(attempts_store[i], attempt, b_h, w_h);
            
            if(b_h != feedback_store[i].black_hits || w_h != feedback_store[i].white_hits)
                return false;
        }
        return true;
    }
    
    void check_highest_feedback(){
        int highest_b_h = feedback_store[index_highest_feedback].black_hits;
        int highest_w_h = feedback_store[index_highest_feedback].white_hits;
        int last_b_h = feedback_store[turn - 1].black_hits;
        int last_w_h = feedback_store[turn - 1].white_hits;
        
        //if the last feedback is more informative we will generate next attempt compatible to this last, instead of the old one.
        if(highest_b_h + highest_w_h < last_b_h + last_w_h)
            index_highest_feedback = turn - 1;
        if (highest_w_h + highest_b_h == last_b_h + last_w_h && last_b_h > highest_b_h)
            index_highest_feedback = turn - 1;
    }
    
    void give_feedback(std::vector<int> possible_soln, std::vector<int> attempt, int& black_hits, int& white_hits){
        black_hits = 0;
        white_hits = 0;
        
        feedback_generator(possible_soln, attempt, black_hits, white_hits, length);
    }
    
    
    void scalable_create_attempt(std::vector<int>& attempt){
        if(turn < num)
            for(int i = 0; i < length; i++)
                attempt.push_back(turn);
        else{
            if(turn == num)
                num_tested = pick_next_num();
            
            if(num_tested == -1)
                for(int i = 0; i < length; i++)
                    attempt.push_back(solution[i]);
            else
                binary_search(attempt);
        }
    }
    
    
    void scalable_learn(std::vector<int>& attempt, int black_hits, int white_hits){
        if(turn <= num){
            appearences.push_back(black_hits);
            if(black_hits == 0)
                mask = turn - 1;
        }
        else{
            if(binary_search_index_end - binary_search_index_start == 1 && black_hits == 1){
                solution[binary_search_index_start] = num_tested;
                
                appearences[num_tested]--;
                
                if(appearences[num_tested] > 0){
                    binary_search_index_start++;
                    binary_search_index_end = binary_search_index_start + (length -binary_search_index_start) / 2;
                }
                else {
                    binary_search_index_start = 0;
                    binary_search_index_end = length / 2;
                    num_tested = pick_next_num();
                }
            }
            
            else if(black_hits > 0){
                binary_search_index_end = binary_search_index_start + (binary_search_index_end - binary_search_index_start) / 2;
            }
            else{
                if(binary_search_index_end - binary_search_index_start > 2){
                    int tmp = binary_search_index_start;
                    binary_search_index_start = binary_search_index_end;
                    binary_search_index_end = binary_search_index_start + (binary_search_index_end - tmp) / 2;
                }
                else{
                    binary_search_index_start++;
                    binary_search_index_end++;
                }
            }
        }
    }
    
    int pick_next_num(){
        for(int i = 0; i < appearences.size(); i++)
            if(appearences[i] > 0)
                return i;
        return -1;
    }
    
    void binary_search(std::vector<int>& attempt){
        for(int i = 0; i < length; i++)
            if(i >= binary_search_index_start && i < binary_search_index_end)
                attempt.push_back(num_tested);
            else
                attempt.push_back(mask);
        
    }
    
    void init_solution_vector(){
        for(int i = 0; i < length; i++)
            solution.push_back(-1);
    }
    
    bool what_approach(){
        if(pow(num, length) > pow(11, 10))
            return true;
        return false;
    }
    
    int length;
    int num;
    int turn;
    bool scalable_approach;
    int index_highest_feedback;
    std::vector<std::vector<int> > attempts_store;
    std::vector<feedback> feedback_store;
    
    int num_tested;
    int mask;
    std::vector<int> appearences;
    std::vector<int> solution;
    int binary_search_index_start;
    int binary_search_index_end;

};

/// before the submission you need to remove the main
/// (either delete it or comment it out)
/// otherwise it will intefere with the automated testing

int main(){
    /// our program uses random features so we need to call the function setting a random seed
    set_random_seed();
    
    int length, num;
    std::cout << "enter length of sequence and number of possible values:" << std::endl;
    std::cin >> length >> num;
    double sum = 0;
    double times = 0;
    clock_t tStart = clock();

    while(times < 10){
     
        mm_solver solver;
        /// we declare an object of type mm_solver
    
        solver.init(length, num);
        /// we initialise the values for length and num
    
        mm_code_maker maker;
        /// we declare an object of type mm_code_maker
    
        maker.init(length, num);
        /// we initialise the values for length and num
    
        maker.generate_sequence();
    
        int black_hits = 0, white_hits = 0;
        /// just some number to limit the number of attempts
        int attempts_limit = 6000;
        int attempts = 0;
        while((black_hits < length) && (attempts < attempts_limit)){
           
            std::vector<int> attempt;
            solver.create_attempt(attempt);
            
            
            /// the solver creates an attempt
            
            maker.give_feedback(attempt, black_hits, white_hits);
            
            /*
            // we print the attempt
            std::cout << "attempt: " << std::endl;
            for(int i = 0; i < attempt.size(); i++){
                std::cout << attempt[i] << " ";
            }
            std::cout << std::endl;
            /// we print the feedback
            std::cout << "black pegs: " << black_hits << " " << " white pegs: " << white_hits << std::endl;
            /// we give the feedback to the solver so that it can learn
            */
            solver.learn(attempt, black_hits, white_hits);
            attempts++;
        }
    
        if(black_hits == length){
            std::cout << "the solver has found the sequence in " << attempts << " attempts" << std::endl;
        }
        else{
            std::cout << "after " << attempts << " attempts still no solution" << std::endl;
        }
        std::cout << "the sequence generated by the code maker was:" << std::endl;
        for(int i = 0; i < maker.sequence.size(); i++){
            std::cout << maker.sequence[i] << " ";
        }
        std::cout << std::endl;
     
        sum = sum + attempts;
        times++;
    }
    std::cout <<"sum = " <<sum <<std::endl;
    std::cout <<"times = " <<times <<std::endl;
    double average = sum/times;
    std::cout <<"after " <<times << " " <<"guesses ";
    std::cout <<"average = " <<average <<std::endl;
    
    printf("Time taken: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
    std::cout <<"time average: " <<(double)(clock() - tStart)/CLOCKS_PER_SEC/times;
    return 0;
    
}


void set_random_seed(){
    std::srand(std::time(0));
}

int randn(int n){
    return std::rand() % n;
}

void poss_sol(std::vector<std::vector<int> >& soln, int length, int num, std::vector<int>& one_solution){
    
    if (one_solution.size() == length)
        soln.push_back(one_solution);
    
    else
        for (int i = 0; i < num; i++){
            one_solution.push_back(i);
            poss_sol(soln, length, num, one_solution);
            one_solution.pop_back();
        }
}


//generate a convenient first move
void first_move(std::vector<int>& attempt, int length, int num){
    for(int i = 0; i < length / 2 ; i++)
        attempt.push_back(0);
    while(attempt.size() < length){
        if(num > 1)
            attempt.push_back(1);
        else
            attempt.push_back(0);
    }
}


void feedback_generator(std::vector<int> sequence, std::vector<int> attempt, int& black_hits, int& white_hits, int length){
    //we give priority to black hits
    black_hits = b_h_feedback(sequence, attempt, length);
    white_hits = w_h_feedback(sequence, attempt, length);
}

//give feedback just for black hits
int b_h_feedback(std::vector<int>& sequence, std::vector<int>& attempt, int length){
    int b_h = 0;
    
    for(int i = 0; i < length; i++)
        if (sequence[i] == attempt[i] && attempt[i] != -1){
            b_h++;
            sequence[i] = -1;
            attempt[i] = -1;
        }
    
    return b_h;
}

//give feedback for white hits
int w_h_feedback(std::vector<int>& sequence, std::vector<int>& attempt, int length){
    int w_h = 0;
    for(int i = 0; i < length; i++)
        for(int j = 0; j < length; j++)
            if(sequence[i] == attempt[j] && attempt[j] != -1){
                w_h++;
                sequence[i] = -1;
                attempt[j] = -1;
            }
    return w_h;
}


