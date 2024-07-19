////////////////////
//  Inividual.h - 
//  Author(s): Patrick Price, Andrew O'Kins
//  Company: WSU ISP ASL
////////////////////

#ifndef INDIVIDUAL_H_
#define INDIVIDUAL_H_

template <class T>
struct Individual {
	// The genome associated with the individual.
	T * genome_;

	// The fitness of the individual, this must be assigned with set_fitness
	// default value is -1 (an impossible real fitness value, so used to identify non-evualeted)
	double fitness_;

	// Constructor, genome is initially null and fitness -1
	Individual() {
		this->genome_ = NULL;
		this->fitness_ = -1;
	}

	// Destructor
	~Individual() {
		if (this->genome_ != NULL) {
			delete[] this->genome_;
		}
	}

	// Returns the array(genome) associated with the individual.
	T * genome() const {
		return this->genome_;
	}

	// Returns the fitness associated with the individual.
	const double fitness() const {
		return this->fitness_;
	}

	// Sets the genome to be associated with the individual.
	// Input: new_genome - genome to be set to this individual
	// Output: new_genome is assigned, old genome is deleted
	void set_genome(T * new_genome) {
		if (this->genome_ != NULL) {
			delete[] this->genome_;
		}
		this->genome_ = new_genome;
	}

	// Sets fitness to be associated with the individual.
	// Input: fitness - the fitness to be associated.
	// Output: this->fitness is assigned input value
	void set_fitness(const double fitness){
		this->fitness_ = fitness;
	}

}; // ... class Individual

// Comparative operator
template <typename T>
bool operator<(Individual<T> &a, Individual<T> &b) {
	return a.fitness() < b.fitness();
}

// Perform a swap of individuals (their pointers are swapped, not a deep copy)
template <typename T>
void swapIndividuals(Individual<T> &a, Individual<T> & b) {
	// Swap fitness
	double temp_fit = a.fitness();
	a.set_fitness(b.fitness());
	b.set_fitness(temp_fit);

	T * temp_ptr = a.genome();
	a.genome_ = b.genome();
	b.genome_ = temp_ptr;
}

#endif
