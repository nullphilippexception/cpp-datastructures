#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

// written by Philipp S.

template <typename Key, size_t N = 15> // hier noch �ndern (Zahl eintragen)
class ADS_set {
    
public: // using declarations ----------------------------------------------------------------------------------------
  class Iterator;
  using value_type = Key;
  using key_type = Key;
  using reference = key_type &;
  using const_reference = const key_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = Iterator;
  using key_compare = std::less<key_type>;    // B+-Tree
  using key_equal = std::equal_to<key_type>;  // Hashing
  using hasher = std::hash<key_type>;         // Hashing
  
 public: // structs zur Aushilfe --------------------------------------------------------------------------------------
  struct Hash; // bekommt einen Wert rein, hasht diesen in Abh�ngigkeit von d und nextToSplit, liefert Hashwert und Wert, indem er sie als Instanzvariablen settet
  struct Bucket; // beinhaltet die Objekte, jeder Bucket hat size b, z�hlt Elemente in sich mit und hat ptr auf einen overflow bucket; falls overflow bucket aktiviert wird: splitting!
  struct Overflow; // handelt den Overflow
  
private: // instanzvariablen ---------------------------------------------------------------------------------------- 
  static constexpr size_type b = N; // bucket size 
  size_type buckets; // Anzahl aktiver regul�rer buckets
  size_type curr_size; // Anzahl Elemente, die aktuell in der Datenstruktur eingetragen sind 
  size_type d; // Anzahl Runden
  size_type nts; // nextToSplit Index des Buckets, der als n�chstes gesplittet wird
  Bucket **array; 
  
public:
// Konstruktoren/Destruktoren -------------------------
  ADS_set();                                                           // PH1
  ADS_set(std::initializer_list<key_type> ilist);                      // PH1
  template<typename InputIt> ADS_set(InputIt first, InputIt last) { // PH1 
    buckets = 2;
 	curr_size = 0;
    d = 1;
  	nts = 0;
    array = new Bucket*[2];
  	array[0] = new Bucket; 
  	array[1] = new Bucket; 
    insert(first, last);
  }  
  
  ADS_set(const ADS_set &other) : ADS_set{other.begin(), other.end()} {} // kopierkonstruktor
  
  ~ADS_set() { 
  		for(size_type i {0}; i < buckets; i++) {
  			delete array[i];
		}
		delete[] array;
  }
  

// Kopieroperatoren ------------------------------------------
  ADS_set &operator=(const ADS_set &other) { // aus refimpl
  	if (this == &other) return *this;
    ADS_set tmp{other};
    swap(tmp);
    return *this;
  }
  
  ADS_set &operator=(std::initializer_list<key_type> ilist) { // aus refimpl
    ADS_set tmp{ilist};
    swap(tmp);
    return *this;
  }
  
  // MEINE METHODEN------------------------------------------
  private:
  	size_type hash(const key_type) const;
  	
  public:
  	void handle();
  // MEINE METHODEN ENDE -----------------------------------
  
  // PH1 Methoden mit Definition au�erhalb ---------------------------------------------
  size_type size() const; // definiert au�erhalb                                           
  bool empty() const; // definiert au�erhalb                                          
  void insert(std::initializer_list<key_type> ilist);  // definiert au�erhalb  
  size_type count(const key_type &key) const; // au�erhalb der klasse definiert
  // Ende PH1 Methoden   -------------------------------------------------------------------- 
       
  std::pair<iterator,bool> insert(const key_type &k) {
    key_type key = k; 
  	size_type hashwert = hash(key);
  	if(array[hashwert]->find(key)) { 
  		return std::pair<iterator,bool>(find(key), false);
	}
	insert({key}); // das soll der ilist insert sein
	return std::pair<iterator,bool>(find(key), true);
  }
  
  template<typename InputIt> void insert(InputIt first, InputIt last) { // PH1 - HIER UMSETZEN
	for (auto it{first}; it != last; ++it) {
    	key_type elem = *it; 
        size_type hashwert = hash(elem);
        // HIER AUF APPEND REAGIEREN
        size_type check_insert = array[hashwert][0].append(elem, false);
        // if(check_insert == 0) ; //case ingnorieren, wert gabs schon
        if(check_insert == 1) curr_size++; //neues element eingef�gt, kein split n�tig
        if(check_insert == 2) { //neues element eingef�gt, split n�tig
        	curr_size++;
        	handle();
        }
    }
  } 
  
  void clear() { // nur elemente l�schen, buckets bleiben (meine interpretation)
  		ADS_set other;
  		this->swap(other);
  }
  
  size_type erase(const key_type &k) { 
  	key_type key = k; 
  	size_type hashwert = hash(key);
  	if(array[hashwert]->erase(key)) {
  		curr_size--;
  		return true;
	}
	return false; //else
  }
  
  iterator find(const key_type &k) const {
  	key_type key = k; 
  	size_type hashwert = hash(key);
  	
  	if(array[hashwert]->find(key)) {
		for(size_type i {0}; i < array[hashwert]->size; i++) { // optimierungspotential, wenn man das mit find kombiniert
			if(key_equal{}(key, array[hashwert]->values[i])) { 
				return iterator(array+hashwert, array[hashwert], i, array+buckets); 
			}
		}
		// falls dort noch ned gefunden: checke overflows
		Bucket *b = array[hashwert]->overflow;
		size_type pos_count = array[hashwert]->size; // z�hlt mit auf welcher position der iterator nachher gesetzt werden soll; TEST sonst unten wieder i
		while(true) { // endlosschleife ok, da returnt wird, wenn es einen hit gibt und hit wird durch vorheriges if statement garantiert
			for(size_type i {0}; i < b->size; i++) { // optimierungspotential, wenn man das mit find kombiniert
				if(key_equal{}(key, b->values[i])) { 
					return iterator(array+hashwert, b, i, array+buckets); 
				}
				pos_count++;
			}
			b = b->overflow; // element wurde noch ned gefunden, es muss also noch einen n�chsten overflow geben
		}
	}
	return end(); 
  }
  
  void swap(ADS_set &other) { //aus refimpl �bernommen
      using std::swap;

   	  swap(array, other.array);
      swap(curr_size, other.curr_size);
      swap(buckets, other.buckets);
      swap(nts, other.nts);
      swap(d, other.d);
  }
  
  void dump(std::ostream &o = std::cerr) const {
  	    o   << " buckets: " << buckets << " d: " << d << " nts: " <<  nts << " curr_size: " << curr_size  << " b: " << b <<std::endl;
    	// print elements
    	for(size_type i {0}; i < buckets; i++) {
    		o << " Bucket# :" << i << " ";
    		for(size_type j {0}; j < array[i][0].size; j++) {
    			o << "[" << array[i][0].values[j] << "] ";
			}
			if(array[i][0].overflow) { 
				o<< " -> ";
				for(size_type j {0}; j < array[i][0].overflow->size; j++) {
					o << "[" << array[i][0].overflow->values[j] << "] ";
				}
			}
			o << std::endl;
		}
   }

// Iterator Methoden ------------- 
  const_iterator begin() const { 
  	if(curr_size > 0)  { 
		size_type start_row {0};
		while(array[start_row]->size == 0 && !(array[start_row]->overflow)) { 
			start_row++;
		}
		if(array[start_row]->size == 0) return const_iterator(array+start_row, array[start_row]->overflow, 0, array+buckets);
		return const_iterator(array+start_row, array[start_row], 0, array+buckets); 
	}
  	return end(); // else
  }
  
  const_iterator end() const { 
  	return iterator(array+buckets);
  }

// Operatoren -------------
friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) { // aus refimpl �bernommen
	if (lhs.curr_size != rhs.curr_size) return false;
    for (const auto &k: rhs) if (!lhs.count(k)) return false; 
    return true;
}

friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) { // aus refimpl �bernommen
	return !(lhs == rhs);
}
  
}; // Ende Klasse ADS_set


// ITERATOR *********************************************************************************************************************** PH2
template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {

	// using deklarationen
	public:
  		using value_type = Key;
  		using difference_type = std::ptrdiff_t;
  		using reference = const value_type &;
  		using pointer = const value_type *;
  		using iterator_category = std::forward_iterator_tag;

	// instanzvariablen  	
	private:
		Bucket **curr_row;
		Bucket *curr_buck;
		size_type pos;
		Bucket **end;
		pointer curr_val;
		
	// private methode
	void skip() { 
		if(curr_row == end) return;
		// DEBUGGING
		// TESTING
		while((*curr_row)->size == 0 && !(*curr_row)->overflow && curr_row != end) {
			curr_row++;
			
			if(curr_row == end) return;
			
			if((*curr_row)->size == 0 && (*curr_row)->overflow) {
				pos = 0;
				curr_buck = (*curr_row)->overflow;
				curr_val = curr_buck->values;
				return;
			}
		}

		// TESTING ENDE
		
		if(pos < curr_buck->size-1 && curr_buck->size > 0) { // Fall 1
			curr_val++;
			pos++;
			return;
		}
		//std::cerr << "first if done\n";
		if(curr_buck->overflow) { // Fall 2a
			Bucket *of = curr_buck;
			while(of->overflow) {
				if(of->overflow->size > 0) {
					pos = 0;
					curr_buck = of->overflow;
					curr_val = of->overflow->values;
					return;
				}
				of = of->overflow; // else
			}
		}
		while(curr_row+1 != end) { // Fall 2b
			curr_row++;
			// testing
			while((*curr_row)->size == 0 && !(*curr_row)->overflow && curr_row != end) {
				curr_row++;
				if(curr_row == end) return;
				if((*curr_row)->size == 0 && (*curr_row)->overflow) {
					pos = 0;
					curr_buck = (*curr_row)->overflow;
					curr_val = curr_buck->values;
					return;
				}	
			}
			// ende testing

			curr_buck = (*curr_row); 

			if(*curr_row) { 
				if(curr_buck->size > 0) {
					curr_val = curr_buck->values;
					pos = 0;
					return;
				}
				else if(curr_buck->overflow) {
					Bucket *of = curr_buck->overflow;
					while(of->size == 0 && of->overflow) {
						of = of->overflow;
					}
					if(of->size > 0) { 
						curr_buck = of;
						pos = 0;
						curr_val = of->values;
						return;
					}
				}	
			}

		}
		curr_row++; // ist jetzt end
		
		return;
		
	}

	// konstruktroen
	public:
	explicit Iterator(Bucket **cr, Bucket *cb, size_type p, Bucket **e) {
		curr_row = cr;
		curr_buck = cb;
		pos = p;
		end = e;
		curr_val = cb->values+pos;	
	} // konstruktor
	
	Iterator(Bucket **e) {
		curr_row = e;
		end = e;
		curr_val = nullptr;
	}
	
	Iterator() {
		curr_val = nullptr;
		pos = -1;
		end = nullptr;
		curr_row = nullptr;
		curr_buck = nullptr;
	}
  
	// methoden  
	reference operator*() const {
		return *curr_val;
	}

	pointer operator->() const {
		return curr_val;
	}

	Iterator &operator++() { 
		skip();
		return *this;
	}

	Iterator operator++(int) { // �bernommen von referenzimplementierung
		auto result{*this};
   	 	++*this;
   	 	return result;
	}

	friend bool operator==(const Iterator &lhs, const Iterator &rhs) {
		if(lhs.curr_row == lhs.end && rhs.curr_row == rhs.end && lhs.end == rhs.end) return true; 
		return lhs.curr_val == rhs.curr_val; 
	}
	friend bool operator!=(const Iterator &lhs, const Iterator &rhs) {
		return !(lhs == rhs);
	}
  
}; // Ende Iterator

template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }


// Hilfs-Structs ************************************************************************************************** PH1
template <typename Key, size_t N>
typename ADS_set<Key, N>::size_type ADS_set<Key, N>::hash(const key_type wert) const {
        
		size_type hashwert = 0;
        auto tmp{hasher{}(wert)};
        
        // f�hre den hash aus
        hashwert = tmp%(1<<d);
        int newd = d+1;
        if(hashwert < nts) hashwert = tmp%(1<<newd);
        return hashwert;
}

template <typename Key, size_t N>
struct ADS_set<Key, N>::Bucket {
        key_type *values = new key_type[b];
        size_type size {0};
        Bucket *overflow = nullptr;
        
        // DESTRUKTOR
        ~Bucket() {
        	delete[] values;
        	if(overflow) delete overflow;
		}
        
        size_type append(key_type& k, bool forced) { 
            if(!find(k)) { // doppelte werte werden stillschweigend ignoriert! -> Angabe
                if(size < b) { 
                    values[size] = k;
                    size++;
                }
                else {
                	if(!overflow) {
                		Bucket *ovf = new Bucket; 
            			overflow = ovf;
            			overflow->values[0] = k; 
            			overflow->size++;
            			if(!forced) {
            			    return 2; // insert erfolgreich, split n�tig
						}
					}
					else {
						if(overflow->splitRequest(k)) { // INCREASED AUF JEDEN FALL CURR_SIZE um 1!
							if(!forced) {
								return 2; // insert erfolgreich, split n�tig
							}
						}
					}
                }
            	return 1; //insert war erfolgreich, wenn hier angekommen: kein split n�tig
			}
            // Wert konnte gefunden werden -> kein insert
            return 0;
        }
        
        void overflowCleanUp() {
        	if(overflow) { 
				(*overflow).overflowCleanUp();
				delete overflow;
			}
        	delete[] values;
        	return;
		}

        bool splitRequest(key_type& k) {
        	if(size < b) {
        		values[size] = k;
        		size++;
        		return false;
        	}
        	if(overflow) return overflow->splitRequest(k);
        	//else 
        	Bucket *ovf = new Bucket; 
            overflow = ovf;
            overflow->values[0] = k; 
            overflow->size++;
            return true;
        }
    
        // suche Element im Bucket
        bool find(key_type& k) {
            for(size_type i{0}; i < size; i++) {
                if(key_equal{}(k, values[i])) return true;
            }
            if(overflow) {
				return (*overflow).find(k); // hier was ge�ndert
			}
            else return false;
        }
        
        // ein Element aus Bucket l�schen
        bool erase(key_type& k) { 
        	if(size > 0 && key_equal{}(k, values[(size-1)])) { // zu l�schendes element ist hinterstes element in diesem bucket
				size--;
				return true; // l�schen erfolgreich
			}
			else { // zu l�schendes element ist nicht hinterstes element in diesem bucket
				for(size_type i {0}; i < size - 1 && size > 0; i++) {
					if(key_equal{}(k, values[i])) {
						values[i] = values[(size-1)];
						size--;
						return true; // l�schen erfolgreich
					}
				}
			}
        	if(overflow) { 
        		Bucket *b = overflow; 
        		Bucket *prev = this;
        		bool cont {true};
        		while(cont) { 
					if(b->size > 0 && key_equal{}(k, b->values[(b->size-1)])) { // zu l�schendes element ist hinterstes element in diesem bucket
						b->size--;
						if(b->size == 0 && !(b->overflow)) {
							delete b; // es ist der letzte overflow bucket und er hat kein element mehr -> kann gel�scht werden 
							prev->overflow = nullptr;
						}
						else if(b->size == 0 && b->overflow) {
							prev->overflow = b->overflow; 
							b->overflow = nullptr; 
							delete b; 
						}
						return true; // l�schen erfolgreich
					}
					else { // zu l�schendes element ist nicht hinterstes element in diesem bucket
						for(size_type i {0}; i < b->size - 1 && b->size > 0; i++) {
							if(key_equal{}(k, b->values[i])) {
								b->values[i] = b->values[(b->size-1)];
								b->size--;
								return true; // l�schen erfolgreich
							}
						}
					}
					// element noch ned gefunden: checke n�chsten overflow
				    if(b->overflow) {
				    	prev = b;
						b = b->overflow;
					}
        			else cont = false;
				}
			} // end if(overflow)
        	return false; // element wurde ned gefunden und konnte daher ned gel�scht werden
		}
    
}; // ENDE struct Bucket

// Methode f�r die Rehashes
template <typename Key, size_t N>
void ADS_set<Key, N>::handle() {
        
        	// k�mmere dich um refresh von array
        	size_type rehash_row = nts;
            nts++;
            
            size_type d_mult {1};
        	for(size_type z {0}; z < d; z++) {
        		d_mult *= 2;
			}
        	if(nts == d_mult) { 
            	nts = 0;
            	d++;
        	}
        	
        	Bucket **newarr = new Bucket*[++buckets];
        	
			newarr[rehash_row] = new Bucket;
			newarr[buckets-1] = new Bucket;
        	
        	// finde raus welche zeile rehashed werden muss
        	for(size_type i {0}; i < buckets; i++) { 
            	// fallunterscheidung
            	if(i == rehash_row) { // weil nts schon erh�ht wurde
                	// rehashe regular elements
                	for(size_type j{0}; j < array[i]->size; j++) {
                    	size_type hashwert =  hash(array[i]->values[j]);
                    	newarr[hashwert]->append(array[i]->values[j],true); 
                	}
                	if(array[i]->overflow) {
                    	bool cont {true};
                    	Bucket *b = array[i]->overflow; 
                    	while(cont) {
                        	for(size_type j{0}; j < (*b).size; j++) {
                            	size_type hashwert = hash((*b).values[j]);
                            	newarr[hashwert]->append(((*b).values[j]),true); 
                        	}
                        	if((*b).overflow) {
                            	b = (*b).overflow; 
                        	}
                        	else cont = false; // raus aus der while schleife
                    	} // ende while schleife
                	}
                	// delete[] array[i]->values;
                	delete array[i];
                	
            	} // ende if i = nts
            	else {
                    if(i < buckets - 1 ) {
						newarr[i] = array[i]; 
					}
            	}
        	} // ende for i < buckets
			
        	delete[] array;
        	array = newarr;  
          
    } //ende void handle

// Konstruktoren ADS_set ************************************************************************************************** PH1
template <typename Key, size_t N>
ADS_set<Key, N>::ADS_set() {
  //  static constexpr size_t b = N; // bucket size  -> done at top
  buckets = 2;
  curr_size = 0;
  d = 1;
  nts = 0;
  array = new Bucket*[2];
  array[0] = new Bucket; 
  array[1] = new Bucket;
}

template <typename Key, size_t N>
ADS_set<Key, N>::ADS_set(std::initializer_list<key_type> ilist) : ADS_set{std::begin(ilist),std::end(ilist)} {}


// METHODEN ADS_set ************************************************************************************************** PH1
template <typename Key, size_t N>
typename ADS_set<Key, N>::size_type ADS_set<Key, N>::size() const {
    return curr_size;
}                                            

template <typename Key, size_t N>
bool ADS_set<Key, N>::empty() const {
    return curr_size == 0; 
} 

template <typename Key, size_t N>
void ADS_set<Key, N>::insert(std::initializer_list<key_type> ilist) {
    insert(std::begin(ilist),std::end(ilist));
}                  

template <typename Key, size_t N>
typename ADS_set<Key, N>::size_type ADS_set<Key, N>::count(const key_type& key) const { 
	key_type k = key;
    size_type hashwert = hash(key);
    return array[hashwert][0].find(k);
}

#endif // ADS_SET_H
