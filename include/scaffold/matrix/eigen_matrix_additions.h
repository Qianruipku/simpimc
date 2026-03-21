inline void set_size(int x) { this->derived().resize(x); }
inline void set_size(int x, int y) { this->derived().resize(x,y); }

inline void ones() { this->derived().setOnes(); }
inline void ones(int x) { this->derived().resize(x); this->derived().setOnes(); }
inline void ones(int x, int y) { this->derived.resize(x,y); this->derived().setOnes(); }

void zeros() { this->derived().setZero(); }
void zeros(int x) { this->derived().resize(x); this->derived().setZero(); }
void zeros(int x, int y) { this->derived().resize(x,y); this->derived().setZero(); }

inline double* memptr() { return this->derived().data(); }
