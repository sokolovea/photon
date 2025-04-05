#!/bin/bash

scales=(2 3 5 8)
input_files=("test0.bmp" "test1.bmp" "test2.bmp" "test3.bmp" "test4.bmp" "test5.bmp")

rm -f image/output*.bmp

# Увеличение
for i in "${!input_files[@]}"; do
    for scale in "${scales[@]}"; do
        output_file="image/output${i}_inc_${scale}x.bmp"
        echo "Running: bmp24 -inc $scale image/${input_files[$i]} $output_file"
        ./bmp24 -inc "$scale" "image/${input_files[$i]}" "$output_file"
    done
done

# Уменьшение
for i in "${!input_files[@]}"; do
    for scale in "${scales[@]}"; do
        output_file="image/output${i}_dec_${scale}x.bmp"
        echo "Running: bmp24 -dec $scale image/${input_files[$i]} $output_file"
        ./bmp24 -dec "$scale" "image/${input_files[$i]}" "$output_file"
    done
done
