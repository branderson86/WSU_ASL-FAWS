////////////////////
// Population.h - base class to encapsulate a population in a genetic algorithm and some of the micro-genetic behaviors
// Last edited: 09/13/2021 by Andrew O'Kins
////////////////////

#ifndef POPULATION_H_
#define POPULATION_H_

#include "Individual.h"
#include "BetterRandom.h"	// Randomizer in generateRandomImage() & Crossover()
#include "Utility.h"		// For printLine() & rejoinClear() & generateRandomImage()

#include "threadPool.h"

template <class T>
class Population {
protected:
	// The array of individuals in the population.
	Individual<T>* individuals_;
	// Number of individuals in the population.
	int pop_size_;
	// Number of individuals in the population that are to be kept as elite.
	int elite_size_;
	// Percentage of genome that must be shared for images to be counted as similar
	double accepted_similarity_;
	// genome length for individual images
	int genome_length_;
	// bool to track if multithreading is enabled or not
	bool multiThread_;

	// How many threads this population can use
	int threadCount_;

	// Pointer to thread pool to give jobs for
	threadPool * myThreadPool_;

	// Array to store results of crossovers to determine if a refresh is needed
	bool * same_check;

	// Pointer (or array if multithreading is used) of random number generator being used
	BetterRandom * rng_machines;

public:
	// Constructor
	// Input:
	//	genome_length:		 the image size (genome) for an individual
	//	population_size:	 the number of individuals for the population
	//	elite_size:			 the number of individuals for the population that are kept as elite
	//	accepted_similarity: precentage of similarity to be counted as same between individuals (default 90%)
	//  multiThread:		 enable usage of multithreading (default true)
	// _threadCount:		 when multithread is enabled, defines how many threads this population will use
	//  myThreadPool:		 set the thread pool to be used when multithreading enabled
	Population(int genome_length, int population_size, int elite_size, double accepted_similarity = .9, bool multiThread = true, int _threadCount = std::thread::hardware_concurrency(), threadPool * myThreadPool = NULL){
		this->genome_length_ = genome_length;
		this->accepted_similarity_ = accepted_similarity;
		this->pop_size_ = population_size;
		this->elite_size_ = elite_size;
		this->multiThread_ = multiThread;
		this->threadCount_ = _threadCount;
		this->myThreadPool_ = myThreadPool;

		// Check to see if elite size exceeds the population size, currently just gives warning
		if (this->elite_size_ > this->pop_size_) {
			Utility::printLine("WARNING: Elite size (" + std::to_string(this->elite_size_) + ") of population exceeding population size (" + std::to_string(this->pop_size_) + ")!");
		}
		if (this->multiThread_ == true) {
			// Setting array of RNG machines to use with default cap
			this->rng_machines = new BetterRandom[this->threadCount_];

			if (this->myThreadPool_ == NULL) {
				Utility::printLine("ERROR: No thread pool set for population!");
			}
		}
		else {
			this->rng_machines = new BetterRandom[1];
		}

		this->individuals_ = new Individual<T>[this->pop_size_];
		this->same_check = new bool[this->pop_size_ - this->elite_size_];

		for (int i = 0; i < this->pop_size_; i++) {
			this->individuals_[i].set_genome(Utility::generateRandomImage<T>(this->genome_length_, this->rng_machines));
		}
		Utility::printLine("INFO: Population created!");
	}

	//Destructor - delete individuals and call rejoinClear() to clear ind_threads
	~Population() {
		delete[] this->individuals_;
		delete[] this->same_check;
		delete[] this->rng_machines;
	}

	Individual<T> * getIndividual(int i) {
		if (i < 0 || i > this->getSize()) {
			return NULL;
		}
		return &this->individuals_[i];
	}

	// Get number of individuals in population
	const int getSize() const {
		return this->pop_size_;
	}

	// Get number of elite individuals in population
	const int getEliteSize() const {
		return this->elite_size_;
	}

	// Getter for image of individual at inputted index
	// Input: i - individual at given index (population not guranteed sorted)
	// Output: the image for the individual
	T * getGenome(int i) const {
		return this->individuals_[i].genome();
	}

	// Setter for the fitness of the individual at given index
	// Input:
	//	i - individual at given index (population not guranteed sorted)
	//	fitness - the fitness value to set to individual
	// Output: individual at index i has its set_fitness() called with fitness as input
	void setFitness(const int i, double fitness) {
		this->individuals_[i].set_fitness(fitness);
	}

	// Gett for fitness of an individual in the population
	// Input:
	//	i - individual at index i (population not guranteed sorted)
	const double getFitness(int i) const {
		return this->individuals_[i].fitness();
	}

	// Crosses over information between individual genomes
	// Input:
	//	a - First individual to be crossed over.
	//	b - Second individual to be crossed over.
	//	same_check - boolean will be set to false if the arrays are different.
	//  useMutation - boolean set if to perform mutation or not, defaults to true (enable).
	// Output: returns new genome as result of crossover algorithm
	T * Crossover(const T * a, const  T * b, bool& same_check, const bool useMutation, BetterRandom * rng_machine) const {
		T * temp = new T[this->genome_length_];
		double same_counter = 0; // counter keeping track of how many indices in the genomes are the same
		// Variabales to hold results for easier readiblity or possible adjustments
		bool choice, mutate;
		T mutateVal;
		// For each index in the genome
		for (int i = 0; i < this->genome_length_; i++) {
			// Set booleans
			choice = ((100 * (*rng_machine)()) / RAND_MAX) < 50;
			mutate = ((200 * (*rng_machine)()) / RAND_MAX) == 0;

			// 50% chance of coming from either parent
			if (choice) {
				temp[i] = a[i];
			}
			else {
				temp[i] = b[i];
			}
			// if the values at an index are the same, increment the same counter
			if (a[i] == b[i])	{
				same_counter += 1;
			}
			// mutation occuring if useMutation and at 0.05% chance
			if (mutate && useMutation)	{
				// Set mutate value
				mutateVal = T(((256 * (*rng_machine)()) / RAND_MAX));
				temp[i] = (T)mutateVal;
			}
		} // ... End image creation

		// if the percentage of indices that are the same is less than the accepted similarity, label the two genomes as not the same (same_check = false)
		same_counter /= this->genome_length_;
		if (same_counter < this->accepted_similarity_) {
			same_check = false;
		}
		return temp;
	}

	// Sorts an array of individuals
	//		Currently (August 31 2021) this is using selection sort and shallow copies (the pointers get swapped, not the "actual" genome data)
	// Input:
	//	to_sort - the individuals to be sorted
	//	size - the size of the array to_sort
	// Output: the to_sort pointer now points to array of individuals that are sorted
	void SortIndividuals(Individual<T> * to_sort, int size) {
		int smallest_index = 0;

		for (int i = 0; i < size - 1; i++) {
			smallest_index = i;

			for (int j = i + 1; j < size; j++)	{
				if (to_sort[j].fitness() < to_sort[smallest_index].fitness()) {
					smallest_index = j;
				}
			}
			if (smallest_index != i) {
				swapIndividuals(to_sort[i], to_sort[smallest_index]);
			}
		}
	}

	// Deep copies the genome & resulting image from one individual to another
	// Input:
	//	to - the individual to be copied to
	//	from - the individual copied
	// Output: to is contains deep copy of from
	void DeepCopyIndividual(Individual<T> & to, Individual<T> & from) const {
		to.set_fitness(from.fitness());
		T * temp_genome1 = new T[this->genome_length_];
		T * temp_genome2 = from.genome();
		for (int i = 0; i < this->genome_length_; i++) {
			temp_genome1[i] = temp_genome2[i];
		}
		to.set_genome(temp_genome1);
	}

	// Perform the genetic algorithm to create new individuals for next gneeration
	// virtual method to have child classes define this behavior
	// Output: False if error occurs, otherwise True
	virtual bool nextGeneration() = 0;

}; // ... class Population

#endif
