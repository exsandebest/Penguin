double peng_pow(double a, double b){}
int peng_pow(int a, int b){
    if (b < 0) throw string("Negative degree in operation '**' with integer arguments is not allowed");
    int result = 1;
    while(b) {
       if (b % 2 == 0) {
           b /= 2;
           a *= a;
       } else {
           --b;
           result *= a;
       }
   }
   return result;
}
