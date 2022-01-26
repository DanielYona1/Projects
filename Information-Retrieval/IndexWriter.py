import os
import shutil
import re
from collections import Counter

BLOCK_LEN = 102

class IndexWriter:

    def __init__(self, inputFile=None, dir=None):                                                                       # set paths
        self.input = inputFile
        if not os.path.exists(dir):
            os.mkdir(dir)
        self.index_directory = dir
        dict_freq = {}
        words_set = set()                                                                                               # set of all unique words
        num_of_reviews = 0
        total_words = 0
        list_review_freq = []
        with open(inputFile, "r") as file:                      
            for line in file:
                line = line.lower()
                if "review/text:" in line:
                    num_of_reviews += 1
                    line = line.replace("review/text: ", "")
                    res = re.findall(r'\w+', line)                                                                      # list of the words in the current review
                    words_freq = Counter(res)
                    list_review_freq.append(words_freq)
                    total_words += len(res)
                    line_words_set = set(res)                                                                           # all unique words in the current line
                    words_set.update(line_words_set)
                    for word in line_words_set:                                                                         # count the frequency of the words
                        if word not in dict_freq:
                            dict_freq[word] = 1
                        else:
                            dict_freq[word] += 1
            dict_words = {}
            for i in range(len(list_review_freq)):                                                                      # go over the list of reviewa
                curr_review = list_review_freq[i]
                for ele in list_review_freq[i]:
                    if ele in dict_words:
                        dict_words[ele].append(i+1)                                                                     # save the review id and frequency in the dictionary
                        dict_words[ele].append(curr_review[ele])
                    else:
                        list_words_freq = [i + 1, curr_review[ele]]
                        dict_words[ele] = list_words_freq

            words = sorted(words_set)                                                                                   # sort words in order to check prefixes
            dictionary_str = ""
            blk_word_count = 0                                                                                          # words in block
            common = ""
            num_of_blocks = 1
            dict_blocks_pointers = {}
            dict_prefix = {}
            for i, word in enumerate(words):
                if blk_word_count is 0:                                                                                 # in case this is the first block
                    dict_blocks_pointers['block' + str(num_of_blocks)] = len(dictionary_str)
                    dictionary_str += word                                                                              # put the first word of in the block completely
                    blk_word_count += 1
                else:
                    prev_word = words[i - 1]
                    smaller_size = min(len(word), len(prev_word))
                    dict_prefix[word] = 0
                    for j in range(smaller_size):                                                                       # go over the words (the current and previous one)
                        if prev_word[j] == word[j]:             
                            common += word[j]                                                                           # count common characters
                            dict_prefix[word] += 1
                        else:
                            break
                    word = word.replace(common, "", 1)                                                                  # keep only the non-common characters
                    common = ""
                    dictionary_str += word
                    blk_word_count += 1
                if blk_word_count is 10 and i+1 < len(words):                                                           # in case of the last word in the block
                    num_of_blocks += 1
                    blk_word_count = 0

            f = open(self.index_directory + "/text.dic", "wb")
            encoded_file = open(self.index_directory + "/text.pl", "wb")
            length = len(dictionary_str)
            f.write(length.to_bytes(4, byteorder='big', signed=False))                                                  # 4 bytes for the length of the string
            f.write(dictionary_str.encode('ascii'))                                                                     # encode the string itself
            word_ptr = 0
            counter = len(dictionary_str) + 4
            cursor = 0
            for i in range(1, num_of_blocks+1):                                                                         # go over all blocks

                curr_block = dict_blocks_pointers['block' + str(i)].to_bytes(4, byteorder='big', signed=False)
                f.write(curr_block)                                                                                     # 4 bytes for the pointer of the block
                counter += 4
                last_word = word_ptr + 9                                                                                # 9 bytes for length etc.
                for j in range(word_ptr, word_ptr + 10):                                                                # go over every ten words from the word's list
                    if j < len(words):
                        curr_freq = dict_freq[words[j]].to_bytes(4, byteorder='big', signed=False)
                        f.write(curr_freq)
                        f.write(cursor.to_bytes(4, byteorder='big', signed=False))
                        for index, num in enumerate(dict_words[words[j]]):                                              # go over all the reviews' list of the current word
                            if index == 0:                                                                              # first review, save the pointer
                                coded_num = self.encodeInt2Bytes(num)
                                encoded_file.write(coded_num)
                            elif index % 2 == 0:                                                                        # in case this is the frequency
                                gap = num - dict_words[words[j]][index-2]                                               # save the diff from the next review's pointer
                                coded_num = self.encodeInt2Bytes(gap)
                                encoded_file.write(coded_num)
                            else:
                                coded_num = self.encodeInt2Bytes(num)
                                encoded_file.write(coded_num)
                            cursor += len(coded_num)
                        counter += 4
                        if j != last_word and j != len(words) - 1:                                                      # don't write length to the last word on block or at all
                            f.write(len(words[j]).to_bytes(1, byteorder='big', signed=False))                           # 1 byte for the length
                            counter += 1

                        if words[j] in dict_prefix:                                                                     # in case not the first word on block
                            curr_pref = dict_prefix[words[j]].to_bytes(1, byteorder='big', signed=False)                # 1 byte for the prefix
                            f.write(curr_pref)
                            counter += 1

                word_ptr += 10                                                                                          # move on to the next 10 words

            f.close()
            file_size = os.path.getsize(self.index_directory + "/text.dic")
            f = open(self.index_directory + "/text.dic", "rb")
            string_size = int.from_bytes(f.read(4), byteorder='big')
            blocks_size = file_size - string_size - 4
            f.close()
            if num_of_blocks * BLOCK_LEN - blocks_size > 0:                                                             # in case the last block doesn't contain 10 words
                f = open(dir + "/text.dic", "ab")
                to_add = num_of_blocks * BLOCK_LEN - blocks_size
                zero = 0
                for i in range(to_add):                                                                                 # fill empty spaces with 0
                    to_write = zero.to_bytes(1, byteorder='big', signed=False)
                    f.write(to_write)
            f.close()
            encoded_file.close()
            f = open(self.index_directory + "/reviews_and_words.txt", "w")
            f.write(str(num_of_reviews))
            f.write("\n")
            f.write(str(total_words))

    def removeIndex(self, dir):                                                                                         # delete all directory and its files

        shutil.rmtree(dir, ignore_errors=True)

    def encodeInt2Bytes(self, number):                                                                                  # method to encode the number to bytes

        coded = b''
        while True:
            inBytes = number & 0x7f
            number >>= 7
            if number:                                                                                                  # in case the number is bigger than 127 bianry
                coded += self.tobyte(inBytes | 0x80)
            else:
                coded += self.tobyte(inBytes)
                break
        return coded

    def tobyte(self, b):                                                                                                # convert to bytes
        return bytes((b,))

