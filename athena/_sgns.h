#ifndef ATHENA__SGNS_H
#define ATHENA__SGNS_H


#include "_core.h"
#include <cstddef>
#include <iostream>
#include <vector>
#include <string>
#include <memory>


class SGNSTokenLearner;
class SGNSSentenceLearner;
class SubsamplingSGNSSentenceLearner;
struct SGNSModel;


// Core SGNS implementation.  At training time takes a single input-output
// word pair (and a specification of the desired number of negative
// samples).  Not intended to be called directly (see SGNSSentenceLearner
// instead).

class SGNSTokenLearner {
  std::weak_ptr<SGNSModel> _model;

  public:
    SGNSTokenLearner(): _model() { }
    virtual void reset_word(long word_idx);
    virtual void token_train(size_t target_word_idx, size_t context_word_idx,
                             size_t neg_samples);
    virtual float compute_gradient_coeff(long target_word_idx,
                                         long context_word_idx,
                                         bool negative_sample) const;
    virtual float compute_similarity(size_t word1_idx, size_t word2_idx) const;
    virtual long find_nearest_neighbor_idx(size_t word_idx);
    virtual long find_context_nearest_neighbor_idx(size_t left_context,
                                                   size_t right_context,
                                                   const long *word_ids);
    virtual bool context_contains_oov(const long* ctx_word_ids,
                                      size_t ctx_size) const;
    virtual void set_model(std::shared_ptr<SGNSModel> model) {
      _model = model;
    }
    virtual ~SGNSTokenLearner() { }

    virtual bool equals(const SGNSTokenLearner& other) const;
    virtual void serialize(std::ostream& stream) const;
    static std::shared_ptr<SGNSTokenLearner> deserialize(std::istream& stream);

  private:
    void _context_word_gradient(long target_word, long context_word);
    void _neg_sample_word_gradient(long target_word, long neg_sample_word);
    void _predicted_word_gradient(long target_word, long predicted_word);
    SGNSTokenLearner(const SGNSTokenLearner& s);
};


// Wraps SGNSTokenLearner, providing the logic for training over
// sentences (sequences of overlapping contexts) and also for looping
// over the words within each context.

class SGNSSentenceLearner {
  std::weak_ptr<SGNSModel> _model;
  size_t _neg_samples;
  bool _propagate_retained;

  public:
    SGNSSentenceLearner(size_t neg_samples, bool propagate_retained):
      _model(),
      _neg_samples(neg_samples),
      _propagate_retained(propagate_retained) { }
    virtual void increment(const std::string& word);
    virtual void sentence_train(const std::vector<std::string>& words);
    virtual void set_model(std::shared_ptr<SGNSModel> model) {
      _model = model;
    }
    virtual ~SGNSSentenceLearner() { }

    virtual bool equals(const SGNSSentenceLearner& other) const;
    virtual void serialize(std::ostream& stream) const;
    static std::shared_ptr<SGNSSentenceLearner> deserialize(std::istream& stream);

  private:
    SGNSSentenceLearner(const SGNSSentenceLearner& s);
};


// Wraps SGNSSentenceLearner, subsampling words by frequency
// (as in word2vec) before training.

class SubsamplingSGNSSentenceLearner {
  std::weak_ptr<SGNSModel> _model;
  bool _propagate_discarded;

  public:
    SubsamplingSGNSSentenceLearner(bool propagate_discarded):
      _model(), _propagate_discarded(propagate_discarded) { }
    virtual void sentence_train(const std::vector<std::string>& words);
    virtual void set_model(std::shared_ptr<SGNSModel> model) {
      _model = model;
    }
    virtual ~SubsamplingSGNSSentenceLearner() { }

    virtual bool equals(const SubsamplingSGNSSentenceLearner& other) const;
    virtual void serialize(std::ostream& stream) const;
    static std::shared_ptr<SubsamplingSGNSSentenceLearner> deserialize(std::istream& stream);

  private:
    SubsamplingSGNSSentenceLearner(const SubsamplingSGNSSentenceLearner& s);
};


struct SGNSModel {
  std::shared_ptr<WordContextFactorization> factorization;
  std::shared_ptr<SamplingStrategy> neg_sampling_strategy;
  std::shared_ptr<LanguageModel> language_model;
  std::shared_ptr<SGD> sgd;
  std::shared_ptr<ContextStrategy> ctx_strategy;
  std::shared_ptr<SGNSTokenLearner> token_learner;
  std::shared_ptr<SGNSSentenceLearner> sentence_learner;
  std::shared_ptr<SubsamplingSGNSSentenceLearner> subsampling_sentence_learner;

  SGNSModel(
      std::shared_ptr<WordContextFactorization> factorization_,
      std::shared_ptr<SamplingStrategy> neg_sampling_strategy_,
      std::shared_ptr<LanguageModel> language_model_,
      std::shared_ptr<SGD> sgd_,
      std::shared_ptr<ContextStrategy> ctx_strategy_,
      std::shared_ptr<SGNSTokenLearner> token_learner_,
      std::shared_ptr<SGNSSentenceLearner> sentence_learner_,
      std::shared_ptr<SubsamplingSGNSSentenceLearner>
        subsampling_sentence_learner_):
          factorization(factorization_),
          neg_sampling_strategy(neg_sampling_strategy_),
          language_model(language_model_),
          sgd(sgd_),
          ctx_strategy(ctx_strategy_),
          token_learner(token_learner_),
          sentence_learner(sentence_learner_),
          subsampling_sentence_learner(subsampling_sentence_learner_) { }

  virtual void serialize(std::ostream& stream) const;
  static std::shared_ptr<SGNSModel> deserialize(std::istream& stream);
  virtual bool equals(const SGNSModel& other) const;
};


#endif
