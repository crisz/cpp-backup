#include "signup.h"
void signup(UserSession& us) {
    ClientCommand c;
    auto signup = c.signup(us.username, us.password);
    bool signup_result = signup.get();

    if (!signup_result) die("Signup failed!");

    std::cout<<"La registrazione è andata a buon fine! Benvenuto!\n"
               "Effettua l'accesso per accedere al servizio."<< std::endl;
}