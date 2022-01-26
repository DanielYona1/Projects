import re
import math

ALPHA = 0.5

class IndexSearcher:
    def __init__(self, reader):
        self.reader = reader

    def vectorSpaceSearch(self, query, k):                                                                              # method to return a k-length list of the most highly ranked matching documents to the given query
        query = query.lower()
        query_words = re.findall(r'\w+', query)
        num_of_words_query = len(query_words)
        set_of_words = set(query_words)
        word_times = {}
        num_of_reviews = self.reader.getNumberOfReviews()
        for word in set_of_words:                                                                                       # count the words in the query
            word_times[word] = query.count(word)
        max_word_query = word_times[max(word_times, key=word_times.get)]
        dict_score_aug = {}
        dict_score_standard = {}
        for word in word_times:                                                                                         # go over the words, calculate its augmented and standard score and put in the related dictionary
            score = (((1-ALPHA)*word_times[word])/max_word_query) + ALPHA
            dict_score_aug[word] = score
            freq = self.reader.getTokenFrequency(word)
            if freq == 0:                                                                                               # in case the word doesnt exist
                dict_score_standard[word] = freq
                continue
            score_std = math.log10(int(num_of_reviews) / int(freq))
            dict_score_standard[word] = score_std

        dict_final_score_query = {}
        for word in set_of_words:                                                                                       # go over all words and calculate the final score (multiply and normalize)
            score = (dict_score_standard[word] * dict_score_aug[word]) * (1/num_of_words_query)
            dict_final_score_query[word] = score

        ##########################################################################################                      # deal with doc
        term_per_doc = {}
        for word in set_of_words:
            doc_score = {}
            reviews_with_token = self.reader.getReviewsWithToken(word)

            for i, freq in enumerate(reviews_with_token):                                                               # go over the documents and count the token's frequencies
                if i % 2 != 0:                                                                                          # the even cells holds the frequencies
                    doc_score[reviews_with_token[i-1]] = (1 + math.log10(freq)) * dict_score_standard[word]

            term_per_doc[word] = doc_score                                                                              # the value of the word is a dictionary of the doc's num and its score
        doc_scores = {}
        for i in range(int(num_of_reviews)):                                                                            # go over all documents and put their scores
            doc_scores[i] = 0
            for word in set_of_words:
                if i in term_per_doc[word]:
                    doc_scores[i] += dict_final_score_query[word] * term_per_doc[word][i]

        doc_scores = {x: y for x, y in doc_scores.items() if y != 0}                                                    # get rid of the 0 scores
        return tuple(sorted(doc_scores, key=doc_scores.get, reverse=True)[:k])                                            # return the k-highest scored documents

