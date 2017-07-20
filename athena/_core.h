#ifndef ATHENA__CORE_H
#define ATHENA__CORE_H


#include <cstddef>
#include <cstring>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <random>
#include <memory>
#include <algorithm>

#include "_math.h"


// frequent-word subsampling threshold as defined in word2vec.
#define DEFAULT_SUBSAMPLE_THRESHOLD 1e-3
#define DEFAULT_VOCAB_DIM 16000
#define DEFAULT_EMBEDDING_DIM 200
#define DEFAULT_REFRESH_INTERVAL 64000
#define DEFAULT_REFRESH_BURN_IN 32000
#define DEFAULT_RESERVOIR_SIZE 100000000

#define ALIGN_EACH_EMBEDDING 1


// pair comparators

template <typename T, typename U>
bool pair_first_cmp(std::pair<T,U> x, std::pair<T,U> y) {
  return x.first < y.first;
}

template <typename T, typename U>
bool pair_second_cmp(std::pair<T,U> x, std::pair<T,U> y) {
  return x.second < y.second;
}


// Language model implemented naively

class NaiveLanguageModel {
  float _subsample_threshold;
  size_t _size;
  size_t _total;
  std::vector<size_t> _counters;
  std::unordered_map<std::string,long> _word_ids;
  std::vector<std::string> _words;

  public:
    NaiveLanguageModel(float subsample_threshold = DEFAULT_SUBSAMPLE_THRESHOLD);
    // return ejected (index, word) pair
    // (index is -1 if nothing was ejected)
    std::pair<long,std::string> increment(const std::string& word);
    // return index of word (-1 if does not exist)
    long lookup(const std::string& word) const;
    // return word at index (raise exception if does not exist)
    std::string reverse_lookup(long word_idx) const;
    // return count at word index
    size_t count(long word_idx) const;
    // return counts of all word indices
    std::vector<size_t> counts() const;
    // return ordered (descending) counts of all word indices
    std::vector<size_t> ordered_counts() const;
    // return number of word types present in language model
    size_t size() const;
    // return total number of word tokens seen by language model
    size_t total() const;
    // return true if word should be kept after subsampling
    // (return true with probability
    // sqrt(subsample_threshold / f(word_idx)) where f(word_idx) is the
    // normalized frequency corresponding to word_idx)
    bool subsample(long word_idx) const;
    void truncate(size_t max_size);
    // sort language model words by count (descending)
    void sort();
    ~NaiveLanguageModel() { }

    /*
    virtual bool equals(const LanguageModel& other) const;
    virtual void serialize(std::ostream& stream) const;
    static NaiveLanguageModel* deserialize(std::istream& stream);
    */

    NaiveLanguageModel(float subsample_threshold,
                  size_t size,
                  size_t total,
                  std::vector<size_t>&& counters,
                  std::unordered_map<std::string,long>&& word_ids,
                  std::vector<std::string>&& words):
        _subsample_threshold(subsample_threshold),
        _size(size),
        _total(total),
        _counters(std::forward<std::vector<size_t> >(counters)),
        _word_ids(
          std::forward<std::unordered_map<std::string,long> >(word_ids)),
        _words(std::forward<std::vector<std::string> >(words)) { }
    NaiveLanguageModel(NaiveLanguageModel&& other):
        _subsample_threshold(other._subsample_threshold),
        _size(other._size),
        _total(other._total),
        _counters(std::move(other._counters)),
        _word_ids(
          std::move(other._word_ids)),
        _words(std::move(other._words)) { }

  private:
    NaiveLanguageModel(const NaiveLanguageModel& lm);
};


// Language model implemented on SpaceSaving approximate counter.

class SpaceSavingLanguageModel {
  float _subsample_threshold;
  size_t _num_counters;
  size_t _size;
  size_t _total;
  size_t _min_idx;
  std::vector<size_t> _counters;
  std::unordered_map<std::string,long> _word_ids;
  std::vector<long> _internal_ids;
  std::vector<long> _external_ids;
  std::vector<std::string> _words;

  public:
    SpaceSavingLanguageModel(
      size_t num_counters = DEFAULT_VOCAB_DIM,
      float subsample_threshold = DEFAULT_SUBSAMPLE_THRESHOLD);
    // return ejected (index, word) pair
    // (index is -1 if nothing was ejected)
    std::pair<long,std::string> increment(const std::string& word);
    // return index of word (-1 if does not exist)
    long lookup(const std::string& word) const;
    // return word at index (raise exception if does not exist)
    std::string reverse_lookup(long ext_word_idx) const;
    // return count at word index
    size_t count(long ext_word_idx) const;
    // return counts of all word indices
    std::vector<size_t> counts() const;
    // return ordered (descending) counts of all word indices
    std::vector<size_t> ordered_counts() const;
    // return number of word types present in language model
    size_t size() const;
    // return number of word types possible language model
    size_t capacity() const;
    // return total number of word tokens seen by language model
    size_t total() const;
    // return true if word should be kept after subsampling
    // (return true with probability
    // sqrt(subsample_threshold / f(word_idx)) where f(word_idx) is the
    // normalized frequency corresponding to word_idx)
    bool subsample(long ext_word_idx) const;
    void truncate(size_t max_size);
    ~SpaceSavingLanguageModel() { }

    /*
    virtual bool equals(const LanguageModel& other) const;
    virtual void serialize(std::ostream& stream) const;
    static SpaceSavingLanguageModel* deserialize(std::istream& stream);
    */

    SpaceSavingLanguageModel(float subsample_threshold,
                             size_t num_counters,
                             size_t size,
                             size_t total,
                             size_t min_idx,
                             std::vector<size_t>&& counters,
                             std::unordered_map<std::string,long>&& word_ids,
                             std::vector<long>&& internal_ids,
                             std::vector<long>&& external_ids,
                             std::vector<std::string>&& words):
        _subsample_threshold(subsample_threshold),
        _num_counters(num_counters),
        _size(size),
        _total(total),
        _min_idx(min_idx),
        _counters(std::forward<std::vector<size_t> >(counters)),
        _word_ids(
          std::forward<std::unordered_map<std::string,long> >(word_ids)),
        _internal_ids(std::forward<std::vector<long> >(internal_ids)),
        _external_ids(std::forward<std::vector<long> >(external_ids)),
        _words(std::forward<std::vector<std::string> >(words)) { }
    SpaceSavingLanguageModel(SpaceSavingLanguageModel&& other):
        _subsample_threshold(other._subsample_threshold),
        _num_counters(other._num_counters),
        _size(other._size),
        _total(other._total),
        _min_idx(other._min_idx),
        _counters(std::move(other._counters)),
        _word_ids(std::move(other._word_ids)),
        _internal_ids(std::move(other._internal_ids)),
        _external_ids(std::move(other._external_ids)),
        _words(std::move(other._words)) { }

  private:
    void _update_min_idx();
    std::pair<long,std::string> _unfull_append(const std::string& word);
    std::pair<long,std::string> _full_replace(const std::string& word);
    std::pair<long,std::string> _full_increment(long ext_idx);
    SpaceSavingLanguageModel(const SpaceSavingLanguageModel& sslm);
};


// Word-context matrix factorization model.

class WordContextFactorization {
  size_t _vocab_dim, _embedding_dim, _actual_embedding_dim;
  AlignedVector _word_embeddings, _context_embeddings;

  public:
    WordContextFactorization(size_t vocab_dim = DEFAULT_VOCAB_DIM,
                             size_t embedding_dim = DEFAULT_EMBEDDING_DIM);
    size_t get_embedding_dim() const;
    size_t get_vocab_dim() const;
    float* get_word_embedding(size_t word_idx);
    float* get_context_embedding(size_t word_idx);
    ~WordContextFactorization() { }

    /*
    virtual bool equals(const WordContextFactorization& other) const;
    virtual void serialize(std::ostream& stream) const;
    static WordContextFactorization*
      deserialize(std::istream& stream);
      */

    WordContextFactorization(size_t vocab_dim,
                             size_t embedding_dim,
                             size_t actual_embedding_dim,
                             AlignedVector&& word_embeddings,
                             AlignedVector&& context_embeddings):
        _vocab_dim(vocab_dim),
        _embedding_dim(embedding_dim),
        _actual_embedding_dim(actual_embedding_dim),
        _word_embeddings(std::forward<AlignedVector>(word_embeddings)),
        _context_embeddings(
          std::forward<AlignedVector>(context_embeddings)) { }
    WordContextFactorization(WordContextFactorization&& other):
        _vocab_dim(other._vocab_dim),
        _embedding_dim(other._embedding_dim),
        _actual_embedding_dim(other._actual_embedding_dim),
        _word_embeddings(std::move(other._word_embeddings)),
        _context_embeddings(
          std::move(other._context_embeddings)) { }

  private:
    WordContextFactorization(const WordContextFactorization& wcf);
};


// Stochastic gradient descent parametrization and state.

class SGD {
  size_t _dimension;
  float _tau, _kappa, _rho_lower_bound;
  std::vector<float> _rho;
  std::vector<size_t> _t;

  public:
    SGD(size_t dimension = 1, float tau = 0, float kappa = 0.6,
        float rho_lower_bound = 0);
    void step(size_t dim);
    float get_rho(size_t dim) const;
    void gradient_update(size_t dim, size_t n, const float *g,
                                 float *x);
    void scaled_gradient_update(size_t dim, size_t n, const float *g,
                                        float *x, float alpha);
    void reset(size_t dim);
    ~SGD() { };

    /*
    virtual bool equals(const SGD& other) const;
    virtual void serialize(std::ostream& stream) const;
    static SGD* deserialize(std::istream& stream);
    */

    SGD(size_t dimension,
        float tau,
        float kappa,
        float rho_lower_bound,
        std::vector<float>&& rho,
        std::vector<size_t>&& t):
          _dimension(dimension),
          _tau(tau),
          _kappa(kappa),
          _rho_lower_bound(rho_lower_bound),
          _rho(std::forward<std::vector<float> >(rho)),
          _t(std::forward<std::vector<size_t> >(t)) { }
    SGD(SGD&& other):
          _dimension(other._dimension),
          _tau(other._tau),
          _kappa(other._kappa),
          _rho_lower_bound(other._rho_lower_bound),
          _rho(std::move(other._rho)),
          _t(std::move(other._t)) { }

  private:
    void _compute_rho(size_t dimension);
    SGD(const SGD& sgd);
};


// Uniform sampling strategy for language model.

template <class LanguageModel>
class UniformSamplingStrategy;

template <class LanguageModel>
class UniformSamplingStrategy {
  public:
    UniformSamplingStrategy() { }
    // sample from uniform distribution
    long sample_idx(const LanguageModel& language_model);
    void
      step(const LanguageModel& language_model, size_t word_idx) { }
    void
      reset(const LanguageModel& language_model,
            const CountNormalizer& normalizer) { }

    UniformSamplingStrategy(UniformSamplingStrategy&& other) { }

    /*
    virtual void serialize(std::ostream& stream) const;
    static UniformSamplingStrategy*
      deserialize(std::istream& stream) {
        return new UniformSamplingStrategy();
      }
    */

  private:
    UniformSamplingStrategy(const UniformSamplingStrategy& other);
};


// Empirical sampling strategy for language model.

template <class LanguageModel>
class EmpiricalSamplingStrategy;

template <class LanguageModel>
class EmpiricalSamplingStrategy {
  size_t _refresh_interval;
  size_t _refresh_burn_in;
  CountNormalizer _normalizer;
  AliasSampler* _alias_sampler;
  size_t _t;
  bool _initialized;

  public:
    EmpiricalSamplingStrategy(CountNormalizer&& normalizer,
                              size_t
                                refresh_interval = DEFAULT_REFRESH_INTERVAL,
                              size_t
                                refresh_burn_in = DEFAULT_REFRESH_BURN_IN);
    // if we have taken no more than refresh_burn_in steps
    // or the number of steps since then is a multiple of
    // refresh_interval, refresh (recompute) distribution
    // based on current counts
    void
      step(const LanguageModel& language_model, size_t word_idx);
    // reset distribution according to specified language model, using
    // specified count normalizer (ignore normalizer provided to ctor)
    void
      reset(const LanguageModel& language_model,
            const CountNormalizer& normalizer);
    // sample from (potentially stale) empirical distribution
    // computed by transforming counts via normalizer
    long sample_idx(const LanguageModel& language_model);
    ~EmpiricalSamplingStrategy() { }

    /*
    virtual bool equals(const SamplingStrategy& other) const;
    virtual void serialize(std::ostream& stream) const;
    static EmpiricalSamplingStrategy*
      deserialize(std::istream& stream);
      */

    EmpiricalSamplingStrategy(size_t refresh_interval,
                              size_t refresh_burn_in,
                              CountNormalizer&& normalizer,
                              AliasSampler* alias_sampler,
                              size_t t,
                              bool initialized):
        _refresh_interval(refresh_interval),
        _refresh_burn_in(refresh_burn_in),
        _normalizer(std::move(normalizer)),
        _alias_sampler(alias_sampler),
        _t(t),
        _initialized(initialized) { }

    EmpiricalSamplingStrategy(EmpiricalSamplingStrategy&& other):
        _refresh_interval(other._refresh_interval),
        _refresh_burn_in(other._refresh_burn_in),
        _normalizer(std::move(other._normalizer)),
        _alias_sampler(other._alias_sampler),
        _t(other._t),
        _initialized(other._initialized) {
      other._initialized = false;
      other._alias_sampler = new AliasSampler(std::vector<float>());
    }

  private:
    EmpiricalSamplingStrategy(const EmpiricalSamplingStrategy& other);
};


// Reservoir sampling strategy for language model.

template <class LanguageModel>
class ReservoirSamplingStrategy;

template <class LanguageModel>
class ReservoirSamplingStrategy {
  ReservoirSampler<long> _reservoir_sampler;

  public:
    ReservoirSamplingStrategy(
      ReservoirSampler<long>&& reservoir_sampler):
        _reservoir_sampler(std::move(reservoir_sampler)) { }
    // (randomly) add word to reservoir
    void
      step(const LanguageModel& language_model, size_t word_idx) {
        _reservoir_sampler.insert(word_idx);
      }
    // re-populate reservoir according to language model
    void
      reset(const LanguageModel& language_model,
            const CountNormalizer& normalizer);
    long sample_idx(const LanguageModel& language_model) {
      return _reservoir_sampler.sample();
    }
    ~ReservoirSamplingStrategy() { }
    /*
    virtual bool equals(const SamplingStrategy& other) const;
    virtual void serialize(std::ostream& stream) const;
    static ReservoirSamplingStrategy*
      deserialize(std::istream& stream);
      */

    ReservoirSamplingStrategy(ReservoirSamplingStrategy&& other):
      _reservoir_sampler(std::move(other._reservoir_sampler)) { }

  private:
    ReservoirSamplingStrategy(const ReservoirSamplingStrategy& other);
};


enum context_strategy_t {
  static_ctx,
  dynamic_ctx
};


// Context size strategy (abstract base class).

class ContextStrategy {
  public:
    // return number of words in left and right context (respectively)
    // given there are at most avail_left and avail_right words to the
    // left and right (respectively); return pair (0,0) if no context
    virtual std::pair<size_t,size_t> size(size_t avail_left,
                                          size_t avail_right) const = 0;
    virtual ~ContextStrategy() { }

    /*
    virtual bool equals(const ContextStrategy& other) const { return true; }
    virtual void serialize(std::ostream& stream) const = 0;
    static ContextStrategy* deserialize(std::istream& stream);
    */

  protected:
    ContextStrategy() { }

  private:
    ContextStrategy(const ContextStrategy& context_strategy);
};


// Static context strategy

class StaticContextStrategy : public ContextStrategy {
  size_t _symm_context;

  public:
    StaticContextStrategy(size_t symm_context):
      ContextStrategy(), _symm_context(symm_context) { }
    // return static (fixed) thresholded context
    virtual std::pair<size_t,size_t> size(size_t avail_left,
                                          size_t avail_right) const;
                                          /*
    virtual bool equals(const ContextStrategy& other) const;
    virtual void serialize(std::ostream& stream) const;
    static StaticContextStrategy*
      deserialize(std::istream& stream);
      */
};


// Dynamic context strategy

class DynamicContextStrategy : public ContextStrategy {
  size_t _symm_context;

  public:
    DynamicContextStrategy(size_t symm_context):
      ContextStrategy(), _symm_context(symm_context) { }
    // return dynamic (sampled) thresholded context
    virtual std::pair<size_t,size_t> size(size_t avail_left,
                                          size_t avail_right) const;
                                          /*
    virtual bool equals(const ContextStrategy& other) const;
    virtual void serialize(std::ostream& stream) const;
    static DynamicContextStrategy*
      deserialize(std::istream& stream);
      */
};


//
// UniformSamplingStrategy
//


/*
template <class LanguageModel>
void UniformSamplingStrategy<LanguageModel>::serialize(ostream& stream) const {
  Serializer<int>::serialize(uniform, stream);
}
*/

template <class LanguageModel>
long UniformSamplingStrategy<LanguageModel>::sample_idx(const LanguageModel& language_model) {
  std::uniform_int_distribution<long> d(0, language_model.size() - 1);
  return d(get_urng());
}


//
// EmpiricalSamplingStrategy
//


template <class LanguageModel>
EmpiricalSamplingStrategy<LanguageModel>::EmpiricalSamplingStrategy(
  CountNormalizer&& normalizer,
  size_t refresh_interval,
  size_t refresh_burn_in):
    _refresh_interval(refresh_interval),
    _refresh_burn_in(refresh_burn_in),
    _normalizer(std::move(normalizer)),
    _alias_sampler(new AliasSampler(std::vector<float>())),
    _t(0),
    _initialized(false) {
}

template <class LanguageModel>
long EmpiricalSamplingStrategy<LanguageModel>::sample_idx(
    const LanguageModel& language_model) {
  if (! _initialized) {
    free(_alias_sampler);
    _alias_sampler = new AliasSampler(
      _normalizer.normalize(language_model.counts())
    );
    _initialized = true;
  }
  return _alias_sampler->sample();
}

template <class LanguageModel>
void EmpiricalSamplingStrategy<LanguageModel>::step(
    const LanguageModel& language_model, size_t word_idx) {
  ++_t;
  if ((! _initialized) ||
      _t < _refresh_burn_in ||
      (_t - _refresh_burn_in) % _refresh_interval == 0) {
    free(_alias_sampler);
    _alias_sampler = new AliasSampler(
      _normalizer.normalize(language_model.counts())
    );
    _initialized = true;
  }
}

template <class LanguageModel>
void EmpiricalSamplingStrategy<LanguageModel>::reset(const LanguageModel& language_model,
                                      const CountNormalizer& normalizer) {
  free(_alias_sampler);
  _alias_sampler = new AliasSampler(
    normalizer.normalize(language_model.counts())
  );
  _initialized = true;
}

/*
void EmpiricalSamplingStrategy::serialize(ostream& stream) const {
  Serializer<int>::serialize(empirical, stream);
  Serializer<size_t>::serialize(_refresh_interval, stream);
  Serializer<size_t>::serialize(_refresh_burn_in, stream);
  Serializer<CountNormalizer>::serialize(*_normalizer, stream);
  Serializer<AliasSampler>::serialize(*_alias_sampler, stream);
  Serializer<size_t>::serialize(_t, stream);
  Serializer<bool>::serialize(_initialized, stream);
}

EmpiricalSamplingStrategy*
    EmpiricalSamplingStrategy::deserialize(istream& stream) {
  auto refresh_interval(*Serializer<size_t>::deserialize(stream));
  auto refresh_burn_in(*Serializer<size_t>::deserialize(stream));
  auto normalizer(Serializer<CountNormalizer>::deserialize(stream));
  auto alias_sampler(Serializer<AliasSampler>::deserialize(stream));
  auto t(*Serializer<size_t>::deserialize(stream));
  auto initialized(*Serializer<bool>::deserialize(stream));
  return new EmpiricalSamplingStrategy(
    refresh_interval,
    refresh_burn_in,
    normalizer,
    alias_sampler,
    t,
    initialized
  );
}

bool EmpiricalSamplingStrategy::equals(const SamplingStrategy& other) const {
  const auto& cast_other(
      dynamic_cast<const EmpiricalSamplingStrategy&>(other));
  return
    _refresh_interval == cast_other._refresh_interval &&
    _refresh_burn_in == cast_other._refresh_burn_in &&
    _normalizer->equals(*(cast_other._normalizer)) &&
    _alias_sampler->equals(*(cast_other._alias_sampler)) &&
    _t == cast_other._t &&
    _initialized == cast_other._initialized;
}
*/


//
// ReservoirSamplingStrategy
//


template <class LanguageModel>
void ReservoirSamplingStrategy<LanguageModel>::reset(const LanguageModel& language_model,
                                      const CountNormalizer& normalizer) {
  // we use a deterministic scheme rather than sampling for the sake
  // of speed (in the case where the reservoir sampler is large and
  // the vocabulary is small)
  std::vector<float> weights(normalizer.normalize(language_model.counts()));
  _reservoir_sampler.clear();

  // first insert elements into reservoir proportional to their
  // probability, rounding down; as we do so write the remaining
  // fractional insertion counts back to `weights`
  size_t num_inserted = 0;
  for (size_t word_idx = 0; word_idx < weights.size(); ++word_idx) {
    const float weight(weights[word_idx] * _reservoir_sampler.size());
    for (size_t i = 1; i <= weight; ++i) {
      step(language_model, word_idx);
      ++num_inserted;
    }
    weights[word_idx] = weight - (long) weight;
  }

  // now sort words by their remaining fractional counts
  std::vector<std::pair<size_t,float> > sorted_words(weights.size());
  for (size_t i = 0; i < weights.size(); ++i) {
    sorted_words[i] = std::make_pair(i, weights[i]);
  }
  std::sort(sorted_words.rbegin(), sorted_words.rend(),
         pair_second_cmp<size_t,float>);

  // finally fill reservoir according to those fractional counts
  for (size_t i = 0; num_inserted + i < _reservoir_sampler.size(); ++i) {
    step(language_model, sorted_words[i % sorted_words.size()].first);
  }
}

/*
void ReservoirSamplingStrategy::serialize(ostream& stream) const {
  Serializer<int>::serialize(reservoir, stream);
  Serializer<ReservoirSampler<long> >::serialize(*_reservoir_sampler, stream);
}

ReservoirSamplingStrategy*
    ReservoirSamplingStrategy::deserialize(istream& stream) {
  auto reservoir_sampler(Serializer<ReservoirSampler<long> >::deserialize(stream));
  return new ReservoirSamplingStrategy(reservoir_sampler);
}

bool ReservoirSamplingStrategy::equals(const SamplingStrategy& other) const {
  const auto& cast_other(
      dynamic_cast<const ReservoirSamplingStrategy&>(other));
  return _reservoir_sampler->equals(*(cast_other._reservoir_sampler));
}
*/


#endif
