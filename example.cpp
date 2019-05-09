#include <iostream>
#include "argparse.hpp"
using namespace std;
using namespace ArgParse;

int main(int argc, char* argv[]) {

    ArgumentParser argparser("this is an example program for argpars, a possible command line is ./example -a 1 -b 0.1 0.5 1.0 --chi --delta --epsilon a b c -g true false");
    argparser.AddArgument("-a", "--alpha", "alpha", ValueType::Int, "fixed int val")
        .SetRequired(true).SetNargs(1).SetChoices<int>({0, 1, 2});
    argparser.AddArgument("-b", "--beta", "beta", ValueType::Double, "fixed double list")
        .SetRequired(false).SetNargs(3).SetRange(0, 1);
    argparser.AddArgument("-c", "--chi", "chi", ValueType::Bool, "action->store_true")
        .SetRequired(false).SetAction(ActionType::StoreTrue);
    argparser.AddArgument("-d", "--delta", "delta", ValueType::Bool, "action->store_false")
        .SetRequired(false).SetAction(ActionType::StoreFalse);
    argparser.AddArgument("-e", "--epsilon", "epsilon", ValueType::String, "nargs*")
        .SetRequired(true).SetNargs('*').SetDefault<string>({"a", "b", "c", "d", "e", "f", "g"});
    argparser.AddArgument("-g", "--gamma", "gamma", ValueType::Bool, "nargs+")
        .SetRequired(true).SetNargs('+').SetDefault<bool>({true, false, true, true});
 
    int ret = argparser.Parse(argc, argv);

    if (ret > 0) {
        argparser.PrintHelp();
        return 0;
    } else if (ret < 0) {
        return -1;
    }
    
    fprintf(stdout, "successfully parsed args from command line\n");
    fprintf(stdout, "alpha = %d\n", argparser.Get<int>("alpha"));
    fprintf(stdout, "beta = %f %f %f\n", 
            argparser.GetList<double>("beta")[0],
            argparser.GetList<double>("beta")[1],
            argparser.GetList<double>("beta")[2]);
    fprintf(stdout, "chi = %s\n", argparser.Get<bool>("chi") ? "true" : "false");
    fprintf(stdout, "delta = %s\n", argparser.Get<bool>("delta") ? "true" : "false");

    fprintf(stdout, "epsilon =");
    for (int i = 0; i < argparser.GetList<string>("epsilon").size(); ++i) {
        fprintf(stdout, " %s", argparser.GetList<string>("epsilon")[i].c_str());
    }
    fprintf(stdout, "\n");

    fprintf(stdout, "gamma =");
    for (int i = 0; i < argparser.GetList<bool>("gamma").size(); ++i) {
        fprintf(stdout, " %s", argparser.GetList<bool>("gamma")[i] ? "true": "false");
    }
    fprintf(stdout, "\n");
    
    return 0;
}
