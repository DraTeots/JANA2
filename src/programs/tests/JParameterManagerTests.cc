
// Copyright 2020, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.


#include <JANA/Services/JParameterManager.h>
#include "catch.hpp"

TEST_CASE("JParameterManager::SetDefaultParameter") {

    JParameterManager jpm;


   SECTION("Multiple calls to SetDefaultParameter with same defaults succeed") {

        jpm.SetParameter("testing:dummy_var", 22);

        int x = 44;
        jpm.SetDefaultParameter("testing:dummy_var", x);
        REQUIRE(x == 22);

        int y = 44;
        jpm.SetDefaultParameter("testing:dummy_var", y);
        REQUIRE(y == 22);
    }


    SECTION("Multiple calls to SetDefaultParameter with same defaults succeed, float") {

        float x = 1.1;
        jpm.SetDefaultParameter("small_float", x);
        float y = 1.1;
        jpm.SetDefaultParameter("small_float", y);
        float temp;
        jpm.Parse<float>(jpm.Stringify(1.1f), temp);
        REQUIRE(jpm.Equals(temp, 1.1f));
        jpm.Parse<float>(jpm.Stringify(1.1f), temp);
        REQUIRE(!jpm.Equals(temp, 1.10001f));

        float v = 1.1e20f;
        jpm.SetDefaultParameter("large_float", v);
        float w = 1.1e20f;

        jpm.SetDefaultParameter("large_float", w);
        jpm.Parse<float>(jpm.Stringify(1.1e20f), temp);
        REQUIRE(jpm.Equals(temp, 1.1e20f));
        jpm.Parse<float>(jpm.Stringify(1.1e20f), temp);
        REQUIRE(!jpm.Equals(temp, 1.100001e20f));

        double xx = 1.1;
        jpm.SetDefaultParameter("small_double", xx);
        double yy = 1.1;
        jpm.SetDefaultParameter("small_double", yy);
        
        double tempD;
        jpm.Parse<double>(jpm.Stringify(1.1), tempD);
        REQUIRE(jpm.Equals(tempD, 1.1));
        jpm.Parse<double>(jpm.Stringify(1.1), tempD);
        REQUIRE(!jpm.Equals(tempD, 1.100001));

        double vv = 1.1e50;
        jpm.SetDefaultParameter("large_double", vv);
        double ww = 1.1e50;
        jpm.SetDefaultParameter("large_double", ww);

        jpm.Parse<double>(jpm.Stringify(1.1e20), tempD);
        REQUIRE(jpm.Equals(tempD, 1.1e20));
        REQUIRE(!jpm.Equals(tempD, 1.1000000001e20));
    }


    SECTION("Multiple calls to SetDefaultParameter with different defaults") {

        // If set, the user provided value overrides ALL default values

        jpm.SetParameter("testing:dummy_var", 22);

        int x = 44;
        jpm.SetDefaultParameter("testing:dummy_var", x);
        REQUIRE(x == 22);

        int y = 77;
        jpm.SetDefaultParameter("testing:dummy_var", y);
        REQUIRE(x == 22);


        // If no value set and there are two conflicting defaults, use the _local_ one
        int z = 44;
        jpm.SetDefaultParameter("testing:dummy_var_2", z);
        REQUIRE(z == 44);

        int zz = 77;
        jpm.SetDefaultParameter("testing:dummy_var_2", zz);
        REQUIRE(zz == 77);
    }

    SECTION("Multiple calls to check strings with spaces") {

        // basic string test
        std::string x = "MyStringValue";
        jpm.SetDefaultParameter("testing:dummy_var", x);
        REQUIRE(x == "MyStringValue");

        // string with spaces
        std::string y = "My String Value With Spaces";
        auto p = jpm.SetDefaultParameter("testing:dummy_var2", y);
        REQUIRE(p->GetValue() == "My String Value With Spaces");

        // Stringify returns identical string
        REQUIRE( jpm.Stringify("My String Value With Spaces") == "My String Value With Spaces" );

        // Parse returns identical string
        std::string z = "My String Value With Spaces";
        std::string testString;
        jpm.Parse<std::string>(z,testString);
        REQUIRE( testString == "My String Value With Spaces" );
    }
}


TEST_CASE("JParameterManagerBoolTests") {
    JParameterManager jpm;

    SECTION("'0' parses to false") {
        jpm.SetParameter("test_param", "0");
        bool val = jpm.GetParameterValue<bool>("test_param");
        REQUIRE(val == false);
    }

    SECTION("'1' parses to true") {
        jpm.SetParameter("test_param", "1");
        bool val = jpm.GetParameterValue<bool>("test_param");
        REQUIRE(val == true);
    }

    SECTION("'off' parses to false") {
        jpm.SetParameter("test_param", "off");
        bool val = jpm.GetParameterValue<bool>("test_param");
        REQUIRE(val == false);
    }

    SECTION("'on' parses to true") {
        jpm.SetParameter("test_param", "on");
        bool val = jpm.GetParameterValue<bool>("test_param");
        REQUIRE(val == true);
    }

    SECTION("'true' parses to true") {
        jpm.SetParameter("test_param", "true");
        bool val = jpm.GetParameterValue<bool>("test_param");
        REQUIRE(val == true);
    }

    SECTION("'false' parses to false") {
        jpm.SetParameter("test_param", "false");
        bool val = jpm.GetParameterValue<bool>("test_param");
        REQUIRE(val == false);
    }

    SECTION("Parsing anything else as bool throws an exception") {
        jpm.SetParameter("test_param", "maybe");
        CHECK_THROWS(jpm.GetParameterValue<bool>("test_param"));
    }

    SECTION("Stringify still works") {
        jpm.SetParameter("test_param", false);
        std::string val = jpm.GetParameterValue<std::string>("test_param");
        REQUIRE(val == "0");

        jpm.SetParameter("test_param", true);
        val = jpm.GetParameterValue<std::string>("test_param");
        REQUIRE(val == "1");
    }
}

TEST_CASE("JParameterManager_VectorParams") {
    JParameterManager jpm;

    SECTION("Reading a vector of strings") {
        jpm.SetParameter("test", "simple,whitespace in middle, also with whitespace padding ");
        std::vector<std::string> vals;
        jpm.GetParameter<std::vector<std::string>>("test", vals);

        REQUIRE(vals[0] == "simple");
        REQUIRE(vals[1] == "whitespace in middle");
        REQUIRE(vals[2] == " also with whitespace padding ");
    }
    SECTION("Writing a vector of strings") {
        std::vector<std::string> inputs;
        inputs.emplace_back("first");
        inputs.emplace_back("second one");
        inputs.emplace_back(" third one ");

        jpm.SetDefaultParameter("test", inputs);
        std::vector<std::string> outputs;
        auto param = jpm.GetParameter("test", outputs);
        REQUIRE(param->GetValue() == "first,second one, third one ");
    }
    SECTION("Reading a vector of ints") {
        jpm.SetParameter("test", "1,2, 3 ");
        std::vector<int32_t> vals;
        jpm.GetParameter("test", vals);

        REQUIRE(vals[0] == 1);
        REQUIRE(vals[1] == 2);
        REQUIRE(vals[2] == 3);
    }
    SECTION("Writing a vector of ints") {
        std::vector<int32_t> inputs;
        inputs.emplace_back(22);
        inputs.emplace_back(49);
        inputs.emplace_back(42);

        jpm.SetDefaultParameter("test", inputs);
        std::vector<std::string> outputs;
        auto param = jpm.GetParameter("test", outputs);
        REQUIRE(param->GetValue() == "22,49,42");
    }
    SECTION("Reading a vector of floats") {
        jpm.SetParameter("test", "1,2,3");
        std::vector<float> vals;
        jpm.GetParameter("test", vals);

        REQUIRE(vals[0] == 1.0f);
        REQUIRE(vals[1] == 2.0f);
        REQUIRE(vals[2] == 3.0f);
    }
    SECTION("Writing a vector of floats") {
        std::vector<float> inputs;
        inputs.emplace_back(22.0);
        inputs.emplace_back(49.2);
        inputs.emplace_back(42.0);

        jpm.SetDefaultParameter("test", inputs);
        std::vector<float> outputs;
        auto param = jpm.GetParameter("test", outputs);
        REQUIRE(param->GetValue() == "22,49.2,42");
    }
}

TEST_CASE("JParameterManager::RegisterParameter") {

    JParameterManager jpm;

    SECTION("Set/Get") {
        int x_default = 44;
        auto x_actual = jpm.RegisterParameter("testing:dummy_var", x_default);
        REQUIRE(x_actual == x_default);
    }

    SECTION("Set/Get templated float") {
        auto y_actual = jpm.RegisterParameter("testing:dummy_var2", 22.0);
        REQUIRE(y_actual == 22.0);
    }

    SECTION("Set/Get default") {
        jpm.SetParameter("testing:dummy_var", 22);
        auto x_actual = jpm.RegisterParameter("testing:dummy_var", 44);  // this should set the default value to 44 while keeping value at 22
        auto x_default_str = jpm.FindParameter("testing:dummy_var")->GetDefault();
        int x_default;
        jpm.Parse<int>(x_default_str,x_default);
        REQUIRE(x_actual == 22);
        REQUIRE(x_default == 44);
    }

}

TEST_CASE("JParameterManager_ArrayParams") {
    JParameterManager jpm;

    SECTION("Reading a array of strings") {
        jpm.SetParameter("test", "simple,whitespace in middle, also with whitespace padding ");
        std::array<std::string,3> vals;
        jpm.GetParameter<std::array<std::string,3>>("test", vals); 
        REQUIRE(vals[0] == "simple");
        REQUIRE(vals[1] == "whitespace in middle");
        REQUIRE(vals[2] == " also with whitespace padding ");
    }
    SECTION("Writing a array of strings") {
        std::array<std::string,3> inputs = {"first", "second one" , " third one "};
        jpm.SetDefaultParameter("test", inputs);
        std::array<std::string,3> outputs;
        auto param = jpm.GetParameter("test", outputs);
        REQUIRE(param->GetValue() == "first,second one, third one ");
    }
    SECTION("Reading a array of ints") {
        jpm.SetParameter("test", "1,2, 3 ");
        std::array<int32_t,3> vals;
        jpm.GetParameter("test", vals);

        REQUIRE(vals[0] == 1);
        REQUIRE(vals[1] == 2);
        REQUIRE(vals[2] == 3);
    }
    SECTION("Writing a array of ints") {
        std::array<int32_t,3> inputs = {22,49,42};
        jpm.SetDefaultParameter("test", inputs);
        std::array<std::string,3> outputs;
        auto param = jpm.GetParameter("test", outputs);
        REQUIRE(param->GetValue() == "22,49,42");
    }
    SECTION("Reading a array of floats") {
        jpm.SetParameter("test", "1,2,3");
        std::array<float,3> vals;
        jpm.GetParameter("test", vals);

        REQUIRE(vals[0] == 1.0f);
        REQUIRE(vals[1] == 2.0f);
        REQUIRE(vals[2] == 3.0f);
    }
    SECTION("Writing a array of floats") {
        std::array<float,3> inputs = {22.0,49.2,42.0};
        jpm.SetDefaultParameter("test", inputs);
        std::array<float,3> outputs;
        auto param = jpm.GetParameter("test", outputs);
        REQUIRE(param->GetValue() == "22,49.2,42");
    }
}

TEST_CASE("JParameterManager_Replicating_Issue_233") {
    JParameterManager jpm;

    SECTION("Double Test case: 1") {
        const double testVal = 0.00000001;
        std::string result = jpm.Stringify <double>(testVal);
        REQUIRE(result == "0.00000001");
    }
    SECTION("Double Test case: 2") {
        const double testVal = 0.01;
        std::string result = jpm.Stringify <double>(testVal);
        REQUIRE(result == "0.01");
    }
    SECTION("Double Test case: 3") {
        const double testVal = 0.0;
        std::string result = jpm.Stringify <double>(testVal);
        REQUIRE(result == "0.0");
    }

    SECTION("Float Test case: 1") {
        const float testVal = 0.0f;
        std::string result = jpm.Stringify <float>(testVal);
        REQUIRE(result == "0.0");
    }

    SECTION("Float Test case: 2") {
        const float testVal = 0.0001f;
        std::string result = jpm.Stringify <float>(testVal);
        REQUIRE(result == "0.0001");
    }



}


