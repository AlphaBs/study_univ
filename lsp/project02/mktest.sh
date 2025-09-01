#test_copy 만들기
mkdir test_copy
cd test_copy

#test dir 만들기
mkdir test
mkdir test/old
mkdir test/old/oil
mkdir test/new
mkdir test/new/nougat

touch test/old/oatmeal.c
touch test/old/octopus.py
touch test/old/olive.cpp
touch test/old/orange.txt
echo "old" > test/old/oil/on.c
echo "old" > test/old/oil/pasta.txt

sleep 2

touch test/new/naan.cpp
touch test/new/nacho.txt
touch test/new/noodle.c
touch test/new/nut.py
echo "new" > test/new/nougat/on.c
echo "new" > test/new/nougat/pasta.txt

touch test/taco.txt
touch test/tea.py
touch test/toast.c
touch test/tomato.cpp

#test1 만들기
cp -pr test test1

mkdir test1/1_exclude
mkdir test1/2_exclude

touch test1/please_dont_copy_me1.json
touch test1/please_dont_copy_me2.json

#test2, 3 만들기
cp -pr test1 test2
cp -pr test1 test3

#output Directory 만들기
mkdir modioutput

sleep 2

mkdir 1fortest
mkdir 1fortest/c
mkdir 1fortest/py
mkdir 1fortest/txt
mkdir 1fortest/cpp

touch 1fortest/c/noodle.c
touch 1fortest/c/oatmeal.c
touch 1fortest/c/toast.c
echo "output" > 1fortest/c/on.c

touch 1fortest/cpp/naan.cpp
touch 1fortest/cpp/olive.cpp
touch 1fortest/cpp/tomato.cpp

touch 1fortest/py/nut.py
touch 1fortest/py/octopus.py
touch 1fortest/py/tea.py

touch 1fortest/txt/nacho.txt
touch 1fortest/txt/orange.txt
touch 1fortest/txt/taco.txt
echo "output" > 1fortest/txt/pasta.txt

cp -pr 1fortest 2fortest
cp -pr 1fortest 3fortest

rm -rf 2fortest/*/t*
rm -rf 2fortest/c
rm -rf 2fortest/py

rm -rf 3fortest/*/t*
rm -rf 3fortest/cpp
rm -rf 3fortest/txt