/*
use makefile to compile these files, and then run following command to run this project.

srun -N<NODES> -c<CPUS> ./mapreduce JOB_NAME NUM_REDUCER DELAY INPUT_FILENAME CHUNK_SIZE LOCALITY_CONFIG_FILENAME OUTPUT_DIR

*/
#include "scheduler.h"
#include "mapper.h"
#include "worker.h"
#include <mpi.h>
#include <sys/time.h>

time_t getTime()
{
    struct timeval time_now{};
    gettimeofday(&time_now, nullptr);
    time_t msecs_time = (time_now.tv_sec * 1000) + (time_now.tv_usec / 1000);
    return msecs_time;
}


int main(int argc, char** argv)
{
    int rank, size;
    time_t curTime;
    MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    // read imput parameters
    std::string job_name = std::string(argv[1]);
    int num_reducer = std::stoi(argv[2]);
    int delay = std::stoi(argv[3]);
    std::string input_filename = std::string(argv[4]);
    int chunk_size = std::stoi(argv[5]);
    std::string locality_config_filename = std::string(argv[6]);
    std::string output_dir = std::string(argv[7]);

    if (rank == size-1) { // scheduler(job tracker)
        boost::filesystem::create_directories("./" + output_dir);
        std::ofstream out("./" + output_dir + "/" + job_name + "-log.out");  // create log file
        out << getTime() << ",Start_Job," << job_name << "," << num_reducer << ',' << delay << ',' << input_filename << ',' << chunk_size << ',' << locality_config_filename << ',' << output_dir << '\n';
        Scheduler s(size-1, num_reducer);
        MPI_Status status;
        int buf[2], tmp;
        double startTime = MPI_Wtime();
        
        /* Read locality file */
        s.ReadIloc(locality_config_filename);
        /* Start map task */
        while(!s.CheckMapFinished()) {
            /* receive any request from worker */
            MPI_Recv(buf, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status); // tag 0 for map task
            if (buf[0] == -1) {
                /* send task or finish signal */
                tmp = s.SendMapTask(status.MPI_SOURCE);
                if (tmp != -1) {// -1 means send finish signal
                    out << getTime() << ',' << "Dispatch_Map Task," << tmp << ',' << status.MPI_SOURCE << '\n';
                }
            }
            else { // print task finish message
                s.UpdateFinishedMapTask();
                out << getTime() << ',' << "Complete_Map Task," << buf[0] << ',' << status.MPI_SOURCE << ',' << buf[1] << '\n';
            }
            
        }
        /* collect map finished signal from all workers */
        MPI_Barrier(MPI_COMM_WORLD);
        time_t startShuffle = getTime();
        out << startShuffle << ',' << "Start_Shuffle,";
        int totalPairs = s.Shuffle();
        out << totalPairs << '\n';
        out << getTime() << ",Finish_Shuffle," << getTime() - startShuffle << '\n';
        /* Prepare reduce task */
        s.StartReduceTask();
        while(!s.CheckReduceFinished()) {
            /* receive any request from worker */
            MPI_Recv(buf, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status); // tag 1 for reduce task
            if (buf[0] == -1) {
                /* send task or finish signal */
                tmp = s.SendReduceTask(status.MPI_SOURCE);
                if (tmp != -1) {
                    out << getTime() << ',' << "Dispatch_Reduce Task," << tmp << ',' << status.MPI_SOURCE << '\n';
                }
            }
            else {
                s.UpdateFinishedReduceTask();
                out << getTime() << ',' << "Complete_Reduce Task," << buf[0] << ',' << status.MPI_SOURCE << ',' << buf[1] << '\n';
            }
        }
        /* collect map finished signal from all workers */
        MPI_Barrier(MPI_COMM_WORLD);
        std::cout << "finish!\n";
        out << getTime() << ',' << "Finish_Job," << MPI_Wtime() - startTime << '\n';
    } else { // worker (task tracker)
        int buf[2];
        Worker w(rank, input_filename, chunk_size, num_reducer, output_dir, delay, job_name, size);
        /* start threads to handle map task */
        w.MapTask(rank);
        /* request a new task if there's any available thread */
        while(1) {
            while(w.GetAvailableMapNum() == 0); // blocked until there is a thread available.
            buf[0] = -1; // just request a new task
            w.RequestNewMapTask();
            
            MPI_Recv(buf, 2, MPI_INT, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            w.NewMapTask(buf[0], buf[1]);
            /* break the loop if get a finish signal */
            if (buf[0] == -1) {
                break;
            }
        }
        /* wait until all threads finish */
        w.FinishTask();
        /* send finished signal to all nodes */
        MPI_Barrier(MPI_COMM_WORLD);
        
        /* start reduce task */
        w.ReduceTask();
        /* request a newe task if there's any available thread*/
        while(1) {
            while(w.GetAvailableReduceNum() == 0);
            buf[0] = -1; // just request a new task
            w.RequestNewReduceTask();
            MPI_Recv(buf, 2, MPI_INT, size-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            w.NewReduceTask(buf[0], buf[1]);
            if (buf[0] == -1) {
                break;
            }
        }
        w.FinishReduceTask();
        MPI_Barrier(MPI_COMM_WORLD);
    }
    MPI_Finalize();
    return 0;
}