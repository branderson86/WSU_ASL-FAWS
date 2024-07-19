////////////////////
// uGA_Population.h - handler for micro genetic algorithm that inherits from base Population
// Last edited: 08/24/2021 by Andrew O'Kins
////////////////////

#ifndef UGAPOPULATION_H_
#define UGAPOPULATION_H_

#include "Population.h"

template <class T>
class uGAPopulation : public Population<T> {
public:
	// Constructor
	// Input:
	//	genome_length:		 the image size (genome) for an individual
	//	population_size:	 the number of individuals for the population
	//	elite_size:			 the number of individuals for the population that are kept as elite
	//	accepted_similarity: precentage of similarity to be counted as same between individuals (default 90%)
	//  multiThread:		 enable usage of multithreading (default true)
	//  myThreadPool:		 set the thread pool to be used when multithreading enabled
	uGAPopulation(int genome_length, int population_size, int elite_size, double accepted_similarity = .9, bool multiThread = true, int _threadCount = std::thread::hardware_concurrency(), threadPool * myThreadPool = NULL)
		: Population<T>(genome_length, population_size, elite_size, accepted_similarity, multiThread, _threadCount, myThreadPool) {};

	// Starts next generation using fitness of individuals.
	bool nextGeneration() {
		// temp for storing sorted current population
		this->SortIndividuals(this->individuals_, this->pop_size_);
		Individual<T>* pool = this->individuals_;
		// temp for storing new population before storing into this->individuals_
		Individual<T>* temp = new Individual<T>[this->pop_size_];

		// Lambda function to do crossover and assignment in a more readibly approachable way
		// Input: indID - index of location to store individual in temp array
		//		parent1 - index of a parent in sorted_temp
		//		parent2 - index of the other parent in sorted_temp
		//	    threadID - the thread running to help access appropriate RNG machine
		// Captures:
		//		temp - pointer array to store new individuals
		//		pool - pointer to array of sorted individuals to draw parents from
		//		this - pointer to current instance of uGA_Population for accessing Crossover method with mutation disabled
		auto genInd = [temp, pool, this](int indID, int parent1, int parent2, int threadID) {
			temp[indID].set_genome(this->Crossover(pool[parent1].genome(), pool[parent2].genome(), this->same_check[indID], false, &this->rng_machines[threadID]));
		};

		auto genSubGroup = [temp, pool, this, &genInd](const int threadID) {
			int groupSize = this->pop_size_ / this->threadCount_;
			int remainder = this->pop_size_ - groupSize*this->threadCount_;
			int start_index = threadID*groupSize;

			if (remainder != 0) {
				if (threadID < remainder) {
					groupSize++;
					start_index += threadID;
				}
				else {
					start_index += remainder;
				}
			}
			for (int id = start_index; id < start_index + groupSize && id < this->pop_size_; id++) {
				switch (id) {
				case(0) :
					genInd(0, 4, 3, threadID);
					break;
				case(1) :
					genInd(1, 4, 2, threadID);
					break;
				case(2) :
					genInd(2, 3, 2, threadID);
					break;
				case(3) :
					genInd(3, 3, 2, threadID);
					break;
				case(4) :
					// Keeping current best onto next generation
					this->DeepCopyIndividual(temp[4], pool[4]);
					break;
				}
			}

		}; // .. genSubGroup


		// Crossover generation for new population
		if (this->multiThread_) {
			for (int i = 0; i < this->threadCount_; i++) {
				this->myThreadPool_->pushJob(std::bind(genSubGroup, i));
			}
			this->myThreadPool_->wait();
		}
		else {
			genInd(0, 4, 3, 0);
			genInd(1, 4, 2, 0);
			genInd(2, 3, 2, 0);
			genInd(3, 3, 2, 0);
			// Keeping current best onto next generation
			DeepCopyIndividual(temp[4], this->individuals_[4]);
		}

		// Collect the resulting same_check values,
		// if at least one is false (not similar) then the result is set to false
		bool same_check_result = true;
		for (int i = 0; i < (this->pop_size_ - this->elite_size_) && same_check_result; i++) {
			if (this->same_check[i] == false) {
				same_check_result = false;
			}
		}

		// if all of our individuals are labeled similar, replace half of them with new images
		if (same_check_result) {
			// Lambda function for randomizing groups
			// Input: threadID - ID of current thread
			//		  numThreads - total number of threads being launched
			// Captures:
			//		temp - pointer to array of individuals to store new random genomes in
			//		this - pointer to current instance of uGA_Population for accessing GenerateRandomImage method
			auto generateSubGroup = [temp, this](const int threadID, const int numThreads) {
				int groupSize = (this->pop_size_ - 1) / numThreads;
				int remainder = (this->pop_size_ - 1) - groupSize*numThreads;
				int start_index = threadID*groupSize;

				if (remainder != 0) {
					if (threadID < remainder) {
						groupSize++;
						start_index += threadID;
					}
					else {
						start_index += remainder;
					}
				}
				for (int id = start_index; id < start_index + groupSize && id < (this->pop_size_ - 1); id++) {
					temp[id].set_genome(Utility::generateRandomImage<T>(this->genome_length_, &this->rng_machines[threadID]));
				}
			};

			// Calling generate random image for bottom 4 individuals (keeping best)

			if (this->multiThread_) {
				for (int i = 0; i < this->threadCount_; i++) {
					this->myThreadPool_->pushJob(std::bind(generateSubGroup, i, this->threadCount_));
				}
				this->myThreadPool_->wait();
			}
			for (int id = 0; id < this->pop_size_ - 1; id++) {
				temp[id].set_genome(Utility::generateRandomImage<T>(this->genome_length_, this->rng_machines));
			}
		}

		// Assign new population to individuals_
		delete[] this->individuals_;
		this->individuals_ = temp;
		return true; // No issues!
	}	// ... Function nextGeneration

}; // ... class uGAPopulation

#endif
