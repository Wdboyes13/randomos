double sqrt(double n) {
    if (n < 0) return -1.0;
    if (n == 0) return 0.0;

    double guess = n / 2.0;
    double tolerance = 0.00001;
    
    while ((guess * guess - n) > tolerance || (n - guess * guess) > tolerance) {
        guess = (guess + n / guess) / 2.0;
    }
    return guess;
}

double exp(double x) {
    double sum = 1.0;
    double term = 1.0;
    int i = 1;

    while (term > 0.000001 || term < -0.000001) {
        term *= x / i;
        sum += term;
        i++;
        if (i > 100) break;
    }
    return sum;
}

double log(double x) {
    if (x <= 0) return -0.0;
    
    double y = (x - 1.0) / (x + 1.0);
    double y2 = y * y;
    double term = y;
    double sum = term;
    
    int k = 3;
    for (int i = 0; i < 50; i++) {
        term *= y2;
        sum += term / k;
        k += 2;
    }
    return 2.0 * sum;
}

double pow(double base, double x) {
    if (base < 0) return 0.0;
    if (base == 0) return 0.0;
    return exp(x * log(base));
}
