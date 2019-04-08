# LSM-tree

LSM-tree - data structure for storage and retrieval of information on disk. The data storage algorithm can be described as follows:

  1. All data is sorted. Data is stored in regular arrays
  2. Data is stored in multiple arrays and each array has a size limit
  3. ALL arrays have different restrictions and are sorted by the size of limit
  4. When one of the arrays is full, all the data is transferred to the neighboring array larger in size by using the merge procedure
  
# Interface Description
```cpp
class LSMTree final
{
  /*
  * Currently only unit64_t is supported 
  */
  using key = uint64_t;
  /*
  * Constructor that takes a directory where the data storage structure will be located
  */
  explicit LSMTree(const std::string& directory);
  /*
  * Merging data from RAM to disk
  */
  void flush();
  /*
  * Insert key into data structure 
  */
  void insert(const key& value);
  /*
  * Remove key from data structure 
  */
  void remove(const key& value);
  /*
  * Find key in data structure 
  */
  bool find(const key& value) const;
  /*
  * The output operator to display all the data. Should be used with caution because it may take a long time
  */
  friend std::ostream& operator<<(std::ostream &strm, const LSMTree& tree)
};
```

# Examples of Use

```cpp
LSMTree data("./storage");
data.insert(10);
assert(data.find(10));
assert(!data.find(9));
assert(!data.find(11));
data.flush();
assert(data.find(10));
assert(!data.find(9));
assert(!data.find(11));
data.remove(10);
assert(!data.find(10));
assert(!data.find(9));
assert(!data.find(11));
```
