int main() {
    int value = 7;
    {
        int value = 20;
        value = value + 1;
    }
    return value;
}
