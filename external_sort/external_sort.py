#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import random
import heapq


# 假定内存只能一次读取10w integer
def part_sort(data_file):
    temp_list = []

    part_file_index = 0
    threshold = 100000
    part_file_dict = dict()

    for index in range(9):
        fp = open("part_sort_" + str(index), 'w')
        part_file_dict[str(index)] = fp

    data_fp = open(data_file, 'r')
    line = data_fp.readline().strip()
    count = 1

    while line:
        temp_list.append(int(line))
        if not count % threshold:
            temp_list.sort()
            for ele in temp_list:
                part_file_dict[str(part_file_index)].write(str(ele) + '\n')
            part_file_index += 1
            temp_list = []
        line = data_fp.readline().strip()
        count += 1

    # write for remaining element
    temp_list.sort()
    for ele in temp_list:
        part_file_dict[str(part_file_index)].write(str(ele) + '\n')

    for fp in part_file_dict.values():
        fp.close()
    data_fp.close()

    print "Done..."

def generate_file(filename):

    # random.seed(1)
    fp = open(filename, 'w')
    count = 0
    for _ in xrange(900000):
        current_number = random.randint(0,5000000)
        fp.write(str(current_number) + '\n')
    fp.close()
    print "Generate 90w integer range from 1-500w success..."

    part_sort(filename)
    print "Part sort in different file success..."


# 因为内存只能 单次读取10W整数 故将内存分为两部分
# 第一部分为 输入缓冲区（9W） 第二部分为 输出缓冲区（1W）
def external_merge_sort():

    thresh = 5000
    fp_list = []
    files_count = 0
    for file_sort in os.listdir("./"):
        if file_sort.startswith("part_sort_"):
            files_count += 1

    input_buffer = [[] for _ in range(files_count)]
    for index in range(files_count):
        fp = open("part_sort_" + str(index), 'r')
        fp_list.append(fp)

    dst_fp = open("sort_res", 'w')

    for index in range(len(fp_list)):
        count = 0
        current_fp = fp_list[index]
        current_value = (int(current_fp.readline().strip()), index)
        while count < thresh:
            input_buffer[index].append(current_value)
            current_value = (int(current_fp.readline().strip()), index)
            count += 1
        ## important !!!
        input_buffer[index].append(current_value)


    print "Init reader buffer success..."
    heap = []
    part_thresh = 500
    for index in range(len(fp_list)):
        heap += input_buffer[index][:part_thresh]
        for _ in range(part_thresh):
            input_buffer[index].pop(0)
    heapq.heapify(heap)

    '''
    core code! 基于堆的动态数据调整
    '''
    while heap:
        smallest_pair = heapq.heappop(heap)
        value = smallest_pair[0]
        index = smallest_pair[1]

        dst_fp.write(str(value) + "\n")
        if input_buffer[index]:
            ele = input_buffer[index].pop(0)
            heapq.heappush(heap, ele)
            # read ele from file
            line = fp_list[index].readline()
            if line:
                line = (int(line), index)
                input_buffer[index].append(line)


    for fp in fp_list:
        fp.close()
    dst_fp.close()
    print "Done..."


if __name__ == '__main__':
    filename = "number.txt"

    generate_file(filename)
    print "Generate file success"
    print "Begin to external merge sort"
    external_merge_sort()