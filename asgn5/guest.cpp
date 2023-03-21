#include "main.hpp"

using namespace std;


// void evict_sig_handler(int guest_id, struct timespec *ts, int sleep_time){

//     // keep timed_wait till timeout or 

// }

void signal_cleaners(int *id){
    
    pthread_mutex_lock(&signal_mutex);
    use_count++;

    if (use_count == 2*num_hotel_rooms){
        sem_wait(&write_sem);
        cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority << ") is the last guest to enter the hotel!" << endl;
        sem_post(&write_sem);

        // signal the all the cleaner processes to clean the rooms
        pthread_cond_broadcast(&signal_cond);
        pthread_mutex_unlock(&signal_mutex);
        
    }
    pthread_mutex_unlock(&signal_mutex);
}

void *guest_func(void *arg){
    int *id = (int *)arg;
    // sleep(1);
    cout << "Guest thread " << *id << " is running" << endl;

    
    while(1){

        // sleep for a random time b/w [10-20] seconds
        int sleep_time = (rand() % 10) + 1;

        sem_wait(&write_sem);
        cout << "[GT]: Guest " << *id <<  " (p = "<< guests[*id].priority << ") is sleeping for " << sleep_time << " seconds!" << endl;
        sem_post(&write_sem);

        sleep(sleep_time);
        sem_wait(&write_sem);
        cout << "[GT]: Guest " << *id <<  " (p = "<< guests[*id].priority << ") is awake!" << endl;
        sem_post(&write_sem);

        pthread_mutex_lock(&signal_mutex);
        while(use_count == 2*num_hotel_rooms){

            sem_wait(&write_sem);
            cout << "[GT]: Guest " << *id <<  " (p = "<< guests[*id].priority << ") waits for a cleaned room!" << endl;
            sem_post(&write_sem);

            pthread_cond_wait(&signal_cond, &signal_mutex);
            
            // wait for a signal from the cleaner thread that a room has been cleaned
        }
        pthread_mutex_unlock(&signal_mutex);

        // if empty_q is not empty, then there is a room available, so occupy it
        sem_wait(&q1_sem);
        if (!empty_q.empty()){
            room *r = empty_q.front();
            empty_q.pop();
            sem_post(&q1_sem);
            r->curr_occupied = 1;
            r->guest_id = *id;
            r->count = 1;

            pthread_mutex_lock(&signal_mutex);
            use_count++;
            pthread_mutex_unlock(&signal_mutex);

            // Before sleeping , push the room and the guest priority into one_occ_q
            sem_wait(&q2_sem);
            one_occ_q.push(make_pair(r, guests[*id].priority));
            sem_post(&q2_sem);

            // sleep for a random time b/w [10-30] seconds
            sleep_time = (rand() % 21) + 10;
            sem_wait(&write_sem);
            cout << "[GT]: Guest " << *id <<  " (p = "<< guests[*id].priority << ") is occupying room " << r->room_no << " for " << sleep_time << " seconds!" << endl;
            sem_post(&write_sem);

            // sleep(sleep_time);

            // implement sleeping using semaphores with timeout (so that if a high priority guest arrives while the current guest is sleeping, the current guest can be evicted)
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += sleep_time;
            
            int ret = sem_timedwait(&room_sem[r->room_no], &ts);

            struct timespec ts2;
            clock_gettime(CLOCK_REALTIME, &ts2);

            sem_wait(&err_sem);
            if (ret == -1){
                if (errno == ETIMEDOUT){
                    sem_post(&err_sem);
                    cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") leaves room  (full stay)" << r->room_no << endl;
                }
                else{
                    perror("sem_timedwait");
                    sem_post(&err_sem);
                    exit(EXIT_FAILURE);
                }
            }
            else{
                sem_post(&err_sem);

                int time_slept = sleep_time - (ts.tv_sec - ts2.tv_sec);

                sem_wait(&write_sem);
                cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") was evicted from room " << r->room_no << " after " << time_slept << " seconds by a high priority guest!" << endl;
                sem_post(&write_sem);
                continue;
            }
            

            // // check if the current guest was evicted based on the time elapsed
            // struct timespec ts2;
            // clock_gettime(CLOCK_REALTIME, &ts2);
            // if (ts2.tv_sec < ts.tv_sec){
            //     // the current guest was evicted

            //     // time for which current guest was sleeping
            //     int time_slept = sleep_time - (ts.tv_sec - ts2.tv_sec);

            //     sem_wait(&write_sem);
            //     cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") was evicted from room " << r->room_no << " after " << time_slept << " seconds!" << endl;
            //     sem_post(&write_sem);
            //     continue;
            // }

            // After waking, remove the room from one_occ_q, and push it to one_free_q (if it is still present in one_occ_q)
            int flag = 0;
            room *r_temp = new room;
            sem_wait(&q2_sem);
            priority_queue<pair<room *, int>, vector<pair<room *, int>>, myComp> temp_q;
            while(!one_occ_q.empty()){
                pair<room *, int> p = one_occ_q.top();
                one_occ_q.pop();
                if (p.first->room_no != r->room_no){
                    temp_q.push(p);
                }
                else{
                    flag = 1;
                    r_temp = p.first;
                }
            }
            one_occ_q = temp_q;
            sem_post(&q2_sem);

            // if the room was found in the queue, then it means that the room was not occupied by another guest while the current guest was sleeping
            // so, push the room to one_free_q
            if (flag == 1){
                sem_wait(&write_sem);
                cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") leaves room  (full stay)" << r->room_no << endl;
                sem_post(&write_sem);

                sem_wait(&q3_sem);
                r_temp->curr_occupied = 0;
                r_temp->guest_id = -1; 
                // r_temp->count = 1;

                // push the room to one_free_q
                one_free_q.push(r_temp);
                sem_post(&q3_sem);

                // signal to other waiting guests that a new room has been added to the one_free_q using one_free_q_signal
                
                // check how many guests are waiting on one_free_q_signal
                // int sem_value;
                // sem_getvalue(&one_free_q_signal, &sem_value);

                // cout << "Sem value: " << sem_value << endl;
                // // signal only if a guest is waiting
                // if (abs(sem_value) > 0){
                //     sem_post(&one_free_q_signal);
                // }

            }
        }

        else {
            sem_post(&q1_sem);

            // check if one_free_q is not empty
            sem_wait(&q3_sem);
            if (!one_free_q.empty()){
                room *r = one_free_q.front();
                one_free_q.pop();
                sem_post(&q3_sem);
                r->curr_occupied = 1;
                r->guest_id = *id;
                if (r->count == 1){
                    r->count = 2;
                    sem_wait(&write_sem);
                    cout << "Room " << r->room_no << " has been occupied by the 2nd guest" << endl;
                    sem_post(&write_sem);
                }
                else{
                    cout << "Some error occured!" << endl;
                    exit(EXIT_FAILURE);
                }

                // push the room to two_occ_q before starting the sleep
                sem_wait(&q4_sem);
                two_occ_q.push(r);
                sem_post(&q4_sem);

                // sem_wait(&use_count_sem);
                // use_count++;

                // if (use_count == 2*num_hotel_rooms){
                //     sem_wait(&write_sem);
                //     cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority << ") is the last guest to enter the hotel!" << endl;
                //     sem_post(&write_sem);

                //     // signal the all the cleaner processes to clean the rooms
                //     pthread_mutex_lock(&signal_mutex);
                //     pthread_cond_broadcast(&signal_cond);
                //     pthread_mutex_unlock(&signal_mutex);
                 
                // }
                // sem_post(&use_count_sem);

                signal_cleaners(id);

                // sleep for a random time b/w [10-30] seconds
                sleep_time = (rand() % 21) + 10;

                sem_wait(&write_sem);
                cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority << ") is occupying room " << r->room_no << " for " << sleep_time << " seconds!" << endl;
                sem_post(&write_sem);

                // implement sleep using semaphores with timeout (sem_timedwait)

                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += sleep_time;

                int ret = sem_timedwait(&room_sem[r->room_no], &ts);

                struct timespec ts2;
                clock_gettime(CLOCK_REALTIME, &ts2);

                sem_wait(&err_sem);
                if (ret == -1){
                    if (errno == ETIMEDOUT){
                        sem_post(&err_sem);
                        cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") leaves room  (full stay)" << r->room_no << endl;
                    }
                    else{
                        perror("sem_timedwait");
                        sem_post(&err_sem);
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                    sem_post(&err_sem);

                    int time_slept = sleep_time - (ts.tv_sec - ts2.tv_sec);

                    sem_wait(&write_sem);
                    cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") was evicted from room " << r->room_no << " after " << time_slept << " seconds by a cleaner!" << endl;
                    sem_post(&write_sem);
                }

                continue;

            }

            else {
                sem_post(&q3_sem);

                // check if one_occ_q is not empty, then the new guest can evict a lower priority guest from a room
                sem_wait(&q2_sem);
                if (!one_occ_q.empty()){
                    pair <room *, int> p = one_occ_q.top();
                    // one_occ_q.pop();
                    // sem_post(&q2_sem);

                    // if the priority of the guest occupying the room is lower than the priority of the new guest, then the new guest can evict the old guest
                    if (p.second < guests[*id].priority){
                        // evict the first guest from the room
                        
                        one_occ_q.pop();
                        sem_post(&room_sem[p.first->room_no]); // signal to the guest occupying the room that he has been evicted

                        sem_post(&q2_sem);

                        sem_wait(&write_sem);
                        cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority << ") is evicting guest " << p.first->guest_id << " (p = "<< guests[p.first->guest_id].priority <<") from room " << p.first->room_no << endl;
                        sem_post(&write_sem);


                        // change the room attributes
                        p.first->curr_occupied = 1;
                        p.first->guest_id = *id;
                        if (p.first->count == 1){
                            p.first->count = 2;
                            sem_wait(&write_sem);
                            cout << "Room " << p.first->room_no << " has been occupied by the 2nd guest" << endl;
                            sem_post(&write_sem);
                        }
                        else{
                            cout << "Some error occured!" << endl;
                            exit(EXIT_FAILURE);
                        }

                        // push the room to two_occ_q before starting the sleep
                        sem_wait(&q4_sem);
                        two_occ_q.push(p.first);
                        sem_post(&q4_sem);


                        // sem_wait(&use_count_sem);
                        // use_count++;

                        // if (use_count == 2*num_hotel_rooms){
                        //     sem_wait(&write_sem);
                        //     cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority << ") is the last guest to enter the hotel!" << endl;
                        //     sem_post(&write_sem);

                        //     // signal the all the cleaner processes to clean the rooms
                        //     pthread_mutex_lock(&signal_mutex);
                        //     pthread_cond_broadcast(&signal_cond);
                        //     pthread_mutex_unlock(&signal_mutex);
                        
                        // }
                        // sem_post(&use_count_sem);

                        signal_cleaners(id);

                        // sleep for a random time b/w [10-30] seconds
                        sleep_time = (rand() % 21) + 10;
                        sem_wait(&write_sem);
                        cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<") is occupying room " << p.first->room_no << " for " << sleep_time << " seconds!" << endl;
                        sem_post(&write_sem);

                        // implement sleep using semaphores with timeout (sem_timedwait)

                        struct timespec ts;
                        clock_gettime(CLOCK_REALTIME, &ts);
                        ts.tv_sec += sleep_time;

                        int ret = sem_timedwait(&clean_evict_sem[p.first->room_no], &ts);

                        struct timespec ts2;
                        clock_gettime(CLOCK_REALTIME, &ts2);

                        sem_wait(&err_sem);
                        if (ret == -1){
                            if (errno == ETIMEDOUT){
                                sem_post(&err_sem);
                                cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") leaves room  (full stay)" << p.first->room_no << endl;
                            }
                            else{
                                perror("sem_timedwait");
                                sem_post(&err_sem);
                                exit(EXIT_FAILURE);
                            }
                        }
                        else{
                            sem_post(&err_sem);

                            int time_slept = sleep_time - (ts.tv_sec - ts2.tv_sec);

                            sem_wait(&write_sem);
                            cout << "[GT]: Guest " << *id << " (p = "<< guests[*id].priority <<  ") was evicted from room " << p.first->room_no << " after " << time_slept << " seconds by a cleaner!" << endl;
                            sem_post(&write_sem);
                        }

                    }

                    else {
                        // if the priority of the guest occupying the room is higher than the priority of the new guest, then the new guest has to wait
                        sem_post(&q2_sem);
                    }
                }

                else {
                    sem_post(&q2_sem);
                    continue;

                }
            }
        }
    }


    return NULL;
}
