import os
from io import BytesIO

BLOCK_LEN = 102


class SecondIndexReader:

    def __init__(self, dir):

        self.index_directory = dir
    
    # method that gets the pointer first word of the block in the dictionary's string and the pointer of the block itself,
    # returns a dictionary of all words in a block and their frequencies
    def get_block_words(self, index, pointer, strSize, curr_blk, blocks_amount):
        with open(self.index_directory + "/text.dic", "rb") as f:
            next_block = pointer + BLOCK_LEN
            f.seek(next_block + 8)
            first_next_block_ptr_to_file = int.from_bytes(f.read(4), byteorder='big')                         # skip 4 of pointer to block in string + 4 of freq
            f.seek(next_block)
            str_next_block = int.from_bytes(f.read(4), byteorder='big') + 4 # get the pointer to the next block in string
            dict_freq = {}
            for i in range(10):
                if i == 0:                                                  # in case of first word in block
                    pointer += 4                                            # skip the 4 bytes represent the length of the string
                    f.seek(pointer)
                    freq = int.from_bytes(f.read(4), byteorder='big')       # read the frequency
                    pointer += 4
                    ptr_to_file = int.from_bytes(f.read(4), byteorder='big')
                    pointer += 4
                    word_length = int.from_bytes(f.read(1), byteorder='big')
                    pointer += 1
                    index += 4
                    f.seek(index)
                    word = f.read(word_length).decode('ascii')
                    index += word_length
                    dict_freq[word] = [freq]
                    dict_freq[word].append(ptr_to_file)
                    prev_word = word
                elif i < 9:                                                  # words 2-9 in the block
                    if curr_blk == blocks_amount:
                        f.seek(pointer + 9)
                        check = int.from_bytes(f.read(4), byteorder='big')
                        if check == 0:
                            continue
                    f.seek(pointer)
                    freq = int.from_bytes(f.read(4), byteorder='big')
                    pointer += 4
                    ptr_to_file = int.from_bytes(f.read(4), byteorder='big')
                    pointer += 4
                    length = int.from_bytes(f.read(1), byteorder='big')
                    pointer += 1
                    prefix = int.from_bytes(f.read(1), byteorder='big')     # read the prefix
                    pointer += 1
                    word = prev_word[:prefix]                               # get the common characters from the previous word
                    to_read = length - prefix
                    f.seek(index)
                    word += f.read(to_read).decode('ascii')                 # read the non-common and decode
                    dict_freq[prev_word].append(ptr_to_file)
                    prev_word = word
                    index += to_read
                    dict_freq[word] = [freq]
                    dict_freq[word].append(ptr_to_file)
                else:                                                       # in case last word on block
                    f.seek(pointer)
                    freq = int.from_bytes(f.read(4), byteorder='big')
                    pointer += 4
                    if curr_blk == blocks_amount:
                        length = strSize - index
                    else:
                        length = str_next_block - index
                    ptr_to_file = int.from_bytes(f.read(4), byteorder='big')
                    pointer += 4
                    prefix = int.from_bytes(f.read(1), byteorder='big')
                    pointer += 1
                    word = prev_word[:prefix]
                    to_read = length
                    f.seek(index)
                    word += f.read(to_read).decode('ascii')
                    dict_freq[prev_word].append(ptr_to_file)
                    prev_word = word
                    index += to_read
                    dict_freq[word] = [freq]
                    dict_freq[word].append(ptr_to_file)
                    if first_next_block_ptr_to_file != 0:
                        dict_freq[word].append(first_next_block_ptr_to_file)
                    else:
                        file_size = os.path.getsize(self.index_directory + "/text.pl")
                        dict_freq[word].append(file_size)


        return dict_freq


    def decode_stream(self, stream):
        """Read a varint from `stream`"""
        shift = 0
        result = 0
        while True:
            i = self._read_one(stream)
            result |= (i & 0x7f) << shift
            shift += 7
            if not (i & 0x80):
                break

        return result

    def decode_bytes(self, buf):
        """Read a varint from from `buf` bytes"""
        return self.decode_stream(BytesIO(buf))

    def _read_one(self, stream):
        """Read a byte from the file (as an integer)
        raises EOFError if the stream ends while reading bytes.
        """
        c = stream.read(1)
        if c == b'':
            raise EOFError("Unexpected EOF while reading bytes")
        return ord(c)

    def getTokenFrequency(self, token):                                                                                 # get frequency of the given word

        file_size = os.path.getsize(self.index_directory + "/text.dic")
        with open(self.index_directory + "/text.dic", "rb") as f:
            string_size = int.from_bytes(f.read(4), byteorder='big')
            blocks_num = (file_size - string_size - 4) // BLOCK_LEN            # calculate the amount of blocks
            first_block_ptr = 4 + string_size                           # skip the first 4 bytes represent the length of the string
            
            right = blocks_num                                          # last block
            left = 0

            while left <= right:                                        # binary search in blocks
                curr_block = ((left + right) // 2)
                mid = ((left + right) // 2) * BLOCK_LEN + first_block_ptr      # pointer of the middle block in the dictionary
                f.seek(mid)
                middle_ptr = int.from_bytes(f.read(4), byteorder='big') # pointer to the middle block in the string
                dict_words_in_block = self.get_block_words(middle_ptr, mid, string_size+4, curr_block, blocks_num-1) # get dictionary of the words in the block
                if token in dict_words_in_block:
                    return dict_words_in_block[token][0]
                if token > list(dict_words_in_block.keys())[0]:         # compare it to the first word in the block (alphabetically)
                    left = (left + right) // 2 + 1
                else:
                    right = (left + right) // 2 - 1

        return 0

    def getNumberOfReviews(self):
        with open(self.index_directory + "/reviews_and_words.txt", "r") as f:
            lines = f.readlines()
        return lines[0]

    def getTokenSizeOfReviews(self):
        with open(self.index_directory + "/reviews_and_words.txt", "r") as f:
            lines = f.readlines()
        return lines[1]

    def getReviewsWithToken(self, token):
        """Return the number of times that a given
        token (i.e., word) appears in all the reviews
        indexed (with repetitions)
        Returns 0 if there are no reviews containing
        this token"""
        file_size = os.path.getsize(self.index_directory + "/text.dic")
        with open(self.index_directory + "/text.dic", "rb") as f:
            string_size = int.from_bytes(f.read(4), byteorder='big')
            blocks_num = (file_size - string_size - 4) // BLOCK_LEN  # calculate the amount of blocks
            first_block_ptr = 4 + string_size  # skip the first 4 bytes represent the length of the string

            right = blocks_num                  # last block
            left = 0

            reviews = list()
            while left <= right:                                                                                        # binary search in blocks
                curr_block = ((left + right) // 2)
                mid = ((left + right) // 2) * BLOCK_LEN + first_block_ptr                                               # pointer of the middle block in the dictionary
                f.seek(mid)
                middle_ptr = int.from_bytes(f.read(4), byteorder='big')  # pointer to the middle block in the string
                dict_ptr_to_file = self.get_block_words(middle_ptr, mid, string_size + 4, curr_block, blocks_num - 1)  # get dictionary of the words in the block
                if token in dict_ptr_to_file:
                    file_ptr = dict_ptr_to_file[token][1]
                    toRead = dict_ptr_to_file[token][2] - file_ptr
                    with open(self.index_directory + "/text.pl", "rb") as file:
                        file.seek(file_ptr)
                        byte_count = 0
                        while byte_count < toRead:
                            coded = file.read(1)
                            byte_count += 1
                            copy = coded
                            decoded = int.from_bytes(coded, byteorder='big')
                            while (decoded & 0x80 == 0x80) and byte_count < toRead:
                                coded = file.read(1)
                                copy += coded
                                byte_count += 1
                                decoded = int.from_bytes(coded, byteorder='big')

                            reviews.append(self.decode_bytes(copy))
                    for i in range(len(reviews)):
                        if i > 1 and i % 2 == 0:
                            reviews[i] += reviews[i-2]

                    return tuple(reviews)
                if token > list(dict_ptr_to_file.keys())[0]:                                                            # compare it to the first word in the block (alphabetically)
                    left = (left + right) // 2 + 1
                else:
                    right = (left + right) // 2 - 1

        return tuple(reviews)

    def getTokenCollectionFrequency(self, token):
        """Return the number of times that a given
        token (i.e., word) appears in all the reviews
        indexed (with repetitions)
        Returns 0 if there are no reviews containing
        this token"""
        total_freq = 0
        list_of_token = self.getReviewsWithToken(token)
        for i in range(len(list_of_token)):
            if i % 2 != 0:
                total_freq += list_of_token[i]

        return total_freq
