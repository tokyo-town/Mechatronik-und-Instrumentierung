a = 1.2;
b = -0.3;

syms x
y = x - a * exp(b * x);

g = finverse(y)