////////////////////
// A Basic Thread Pool class to manage and give tasks to persistent threads instead of creating individual threads for each
// beginning implementation credited to https://codereview.stackexchange.com/questions/221617/thread-pool-c-implementation for observing a general approach
// For using std::bind to use arbitrary void function that has inputs with this simple class https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
// Last Edited: 09/02/2021 by Andrew O'Kins
////////////////////

#ifndef THREAD_POOL
#define THREAD_POOL

#include <thread>
#include <condition_variable>
#include <mutex>

#include <vector> // For storing the threads
#include <queue>  // For storing jobs that need to be done

// A class that holds a persistant pool of threads to have perform functions (jobs) with
// Each "job" must be a void function with no inputs (for functions that have inputs, using "bind" works)
class threadPool {
private:
	std::vector<std::thread> myThreads_;
	// Number of threads for this pool
	int num_threads_;

	// boolean that if false means the threads should be rejoined
	bool run_threads_;

	int num_active_tasks; // How many threads are actively performing a job

	// A queue of functions that are waiting to be executed
	std::queue< std::function<void()> > job_queue;
	// Mutex for the queue
	std::mutex job_queue_mutex, numTasksMutex, waitMutex;

	// Conditional variable to have each thread waiting and listen for when a new job has been added
	std::condition_variable queueListen, waitListen;

	// Private method that will be what each thread in the pool will be doing 
	void mainThreadLoop() {
		std::function<void()> myJob;
		std::unique_lock<std::mutex> countLock(this->numTasksMutex, std::defer_lock);
		while (true) {

			std::unique_lock<std::mutex> notify(this->job_queue_mutex);
			// Wait until the job queue is not empty or if we need to end the loop
			this->queueListen.wait(notify, [this] {return (!this->job_queue.empty() || this->run_threads_ == false); });
			// Check again that the threads are supposed to be continuing
			if (this->run_threads_ == false) {
				break;
			}
			// Get job and remove it from the queue
			if (this->job_queue.size() > 0) {
				myJob = std::move(this->job_queue.front());
				this->job_queue.pop();
			}
			notify.unlock(); // Allow another thread to get into the wait

			// Executing task on this thread
			// Increment count
			countLock.lock();
			++this->num_active_tasks;
			countLock.unlock();
			myJob();
			// Decrement count
			countLock.lock();
			--this->num_active_tasks;
			countLock.unlock();

			// Notify waiters in case this was the last job
			this->waitListen.notify_one();
		}
	};

public:
	threadPool() = delete;
	threadPool(threadPool & other) = delete;
	threadPool& operator=(threadPool & other) = delete;

	// Only allowed constructor
	threadPool(int num_threads) {
		this->num_threads_ = num_threads;
		this->run_threads_ = true;
		this->num_active_tasks = 0;

		this->myThreads_.clear();
		// Setup the threads to use the thread loop function to get jobs
		this->myThreads_.reserve(num_threads);
		for (int i = 0; i < this->num_threads_; i++) {
			this->myThreads_.push_back(std::thread(&threadPool::mainThreadLoop, this));
		}
	}

	// Destructor, rejoins the threads
	~threadPool() {
		this->run_threads_ = false;
		this->queueListen.notify_all();

		for (int i = 0; i < this->myThreads_.size(); i++) {
			if (this->myThreads_[i].joinable()) {
				this->myThreads_[i].join();
			}
		}
		this->myThreads_.clear();

		this->waitListen.notify_all();
	}

	// Add a new job to the queue for the pool to work on
	// Input: new_job - a void function to perform
	// Output: The function is added to the queue and a waiting thread will be notified to take it
	void pushJob(std::function<void()> new_job) {
		// Push a new job onto the queue
		std::unique_lock<std::mutex> queue_lock(this->job_queue_mutex);
		this->job_queue.push(new_job);
		queue_lock.unlock();
		// Notify a thread that is waiting that a new job has been pushed
		this->queueListen.notify_one();
	}

	// A simple wait that will make the caller wait until the queue is empty (timeout to check after a microsecond too)
	void wait() {
		std::unique_lock<std::mutex> busyWaitLock(this->waitMutex);

		do {
			this->waitListen.wait_for(busyWaitLock, std::chrono::microseconds(1), [this](){
				return (!this->isBusy());
			});
		} while (this->isBusy());

		this->waitListen.notify_one();
	}

	// Thread-safe call to see if the threads are busy
	// Will return true if at least one thread is currently executing a job, or if there are jobs still left in the queue to do
	bool isBusy() {
		std::unique_lock<std::mutex> queueLock(this->job_queue_mutex);
		std::unique_lock<std::mutex> tasklock(this->numTasksMutex);
		// Is busy if have active tasks or there are tasks in the queue to be done
		bool busy = this->num_active_tasks != 0 || !this->job_queue.empty();
		tasklock.unlock();
		queueLock.unlock();
		return busy;
	}
};

#endif
