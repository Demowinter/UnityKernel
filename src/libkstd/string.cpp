#include <cstddef>
#include <libkstd/string.hpp>

namespace KernelSTD {
    String::String(const char* str) {

    }

    String::String(const char* str, size_t size) {

    }

    String::String(char ch, size_t size) {

    }

    String::String(const String& obj) {

    }

    String::String(String&& obj) {

    }

    String& String::operator=(const char*) {

    }

    String& String::operator=(const String& obj) {

    }

    String& String::operator=(String&& obj) {

    }

    String& String::operator+=(const char*) {

    }

    String& String::operator+=(const String& obj) {

    }

    char& String::operator[](size_t index) {

    }

    void String::copy(const String&) {

    }

    void String::swap(String&) {

    }

    void String::resize(){

    }

    String::ForwardIterator String::begin() {

    }

    String::ForwardIterator String::end() {

    }


    String::ForwardIterator::ForwardIterator(char* ptr) {
        
    }

    String::ForwardIterator& String::ForwardIterator::operator++() {
        ptr++;

        return *this;
    }

    String::ForwardIterator& String::ForwardIterator::operator--() {
        ptr--;

        return *this;
    }

    String::ForwardIterator String::ForwardIterator::operator++(int) {
        ptr++;

        return ptr - 1;
    }

    String::ForwardIterator String::ForwardIterator::operator--(int) {
        ptr--;

        return ptr + 1;
    }

    String::ForwardIterator& String::ForwardIterator::operator+=(size_t size) {
        ptr += size;

        return *this;
    }

    String::ForwardIterator& String::ForwardIterator::operator-=(size_t size) {
        ptr -= size;

        return *this;
    }

    String::ForwardIterator String::ForwardIterator::operator+(size_t size)  {
        return ptr + size;
    }

    String::ForwardIterator String::ForwardIterator::operator-(size_t size)  {
        return ptr - size;
    }

    char& String::ForwardIterator::operator*() {
        return *base();
    }

    char* String::ForwardIterator::operator->() {
        return base();
    }

    char* String::ForwardIterator::base() { return ptr; }
    const char* String::ForwardIterator::base() const { return ptr; }

    // ReverseIterator...
}