#include <iostream>
#include <fstream>
#include "rrLogger.h"
#include "rrUtils.h"
#include "unit_test/UnitTest++.h"
#include "unit_test/XmlTestReporter.h"
#include "unit_test/TestReporterStdout.h"
#include "rrc_api.h"
#include "rrGetOptions.h"
#include "src/Args.h"
#include "rrRoadRunner.h"
#include "rrConfig.h"

#include "Suite_TestModel.h"

using namespace std;
using namespace rr;
using namespace rrc;
using namespace UnitTest;
using std::string;

string     gTempFolder              = "";
string     gRRInstallFolder         = "";
string     gTestDataFolder          = "";
bool       gDebug                   = false;
string     gTSModelsPath;
string     gCompiler                = "";

void ProcessCommandLineArguments(int argc, char* argv[], Args& args);
bool setup(Args& args);

static const char* feedback_sbml =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<sbml xmlns=\"http://www.sbml.org/sbml/level2\" xmlns:jd2=\"http://www.sys-bio.org/sbml/jd2\" level=\"2\" version=\"1\">\r\n  <model id=\"feedback\" name=\"Feedback\">\r\n    <listOfCompartments>\r\n      <compartment id=\"compartment\" size=\"1\"/>\r\n    </listOfCompartments>\r\n    <listOfSpecies>\r\n      <species id=\"S1\" compartment=\"compartment\" initialConcentration=\"0\"/>\r\n      <species id=\"S2\" compartment=\"compartment\" initialConcentration=\"0\"/>\r\n      <species id=\"S3\" compartment=\"compartment\" initialConcentration=\"0\"/>\r\n      <species id=\"S4\" compartment=\"compartment\" initialConcentration=\"0\"/>\r\n      <species id=\"X0\" compartment=\"compartment\" initialConcentration=\"10\" boundaryCondition=\"true\"/>\r\n      <species id=\"X1\" compartment=\"compartment\" initialConcentration=\"0\" boundaryCondition=\"true\"/>\r\n    </listOfSpecies>\r\n    <listOfParameters>\r\n      <parameter id=\"J0_VM1\" value=\"10\"/>\r\n      <parameter id=\"J0_Keq1\" value=\"10\"/>\r\n      <parameter id=\"J0_h\" value=\"10\"/>\r\n      <parameter id=\"J4_V4\" value=\"2.5\"/>\r\n      <parameter id=\"J4_KS4\" value=\"0.5\"/>\r\n    </listOfParameters>\r\n    <listOfReactions>\r\n      <reaction id=\"J0\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"X0\"/>\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"S1\"/>\r\n        </listOfProducts>\r\n        <listOfModifiers>\r\n          <modifierSpeciesReference species=\"S4\"/>\r\n        </listOfModifiers>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <divide/>\r\n              <apply>\r\n                <times/>\r\n                <ci> J0_VM1 </ci>\r\n                <apply>\r\n                  <minus/>\r\n                  <ci> X0 </ci>\r\n                  <apply>\r\n                    <divide/>\r\n                    <ci> S1 </ci>\r\n                    <ci> J0_Keq1 </ci>\r\n                  </apply>\r\n                </apply>\r\n              </apply>\r\n              <apply>\r\n                <plus/>\r\n                <cn type=\"integer\"> 1 </cn>\r\n                <ci> X0 </ci>\r\n                <ci> S1 </ci>\r\n                <apply>\r\n                  <power/>\r\n                  <ci> S4 </ci>\r\n                  <ci> J0_h </ci>\r\n                </apply>\r\n              </apply>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n      <reaction id=\"J1\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"S1\"/>\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"S2\"/>\r\n        </listOfProducts>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <divide/>\r\n              <apply>\r\n                <minus/>\r\n                <apply>\r\n                  <times/>\r\n                  <cn type=\"integer\"> 10 </cn>\r\n                  <ci> S1 </ci>\r\n                </apply>\r\n                <apply>\r\n                  <times/>\r\n                  <cn type=\"integer\"> 2 </cn>\r\n                  <ci> S2 </ci>\r\n                </apply>\r\n              </apply>\r\n              <apply>\r\n                <plus/>\r\n                <cn type=\"integer\"> 1 </cn>\r\n                <ci> S1 </ci>\r\n                <ci> S2 </ci>\r\n              </apply>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n      <reaction id=\"J2\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"S2\"/>\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"S3\"/>\r\n        </listOfProducts>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <divide/>\r\n              <apply>\r\n                <minus/>\r\n                <apply>\r\n                  <times/>\r\n                  <cn type=\"integer\"> 10 </cn>\r\n                  <ci> S2 </ci>\r\n                </apply>\r\n                <apply>\r\n                  <times/>\r\n                  <cn type=\"integer\"> 2 </cn>\r\n                  <ci> S3 </ci>\r\n                </apply>\r\n              </apply>\r\n              <apply>\r\n                <plus/>\r\n                <cn type=\"integer\"> 1 </cn>\r\n                <ci> S2 </ci>\r\n                <ci> S3 </ci>\r\n              </apply>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n      <reaction id=\"J3\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"S3\"/>\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"S4\"/>\r\n        </listOfProducts>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <divide/>\r\n              <apply>\r\n                <minus/>\r\n                <apply>\r\n                  <times/>\r\n                  <cn type=\"integer\"> 10 </cn>\r\n                  <ci> S3 </ci>\r\n                </apply>\r\n                <apply>\r\n                  <times/>\r\n                  <cn type=\"integer\"> 2 </cn>\r\n                  <ci> S4 </ci>\r\n                </apply>\r\n              </apply>\r\n              <apply>\r\n                <plus/>\r\n                <cn type=\"integer\"> 1 </cn>\r\n                <ci> S3 </ci>\r\n                <ci> S4 </ci>\r\n              </apply>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n      <reaction id=\"J4\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"S4\"/>\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"X1\"/>\r\n        </listOfProducts>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <divide/>\r\n              <apply>\r\n                <times/>\r\n                <ci> J4_V4 </ci>\r\n                <ci> S4 </ci>\r\n              </apply>\r\n              <apply>\r\n                <plus/>\r\n                <ci> J4_KS4 </ci>\r\n                <ci> S4 </ci>\r\n              </apply>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n    </listOfReactions>\r\n  </model>\r\n</sbml>\r\n;";

static const char* bistable_sbml =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<!-- Created by JarnacLite version 1.0.4965.360 on 2013-10-09 15:31 with libSBML version 5.8.0. -->\r\n<sbml xmlns=\"http://www.sbml.org/sbml/level2/version4\" level=\"2\" version=\"4\">\r\n  <model id=\"cell\" name=\"cell\">\r\n    <listOfCompartments>\r\n      <compartment id=\"compartment\" size=\"1\" />\r\n    </listOfCompartments>\r\n    <listOfSpecies>\r\n      <species id=\"Xo\" compartment=\"compartment\" initialConcentration=\"0\" boundaryCondition=\"true\" />\r\n      <species id=\"w\" compartment=\"compartment\" initialConcentration=\"0\" boundaryCondition=\"true\" />\r\n      <species id=\"x\" compartment=\"compartment\" initialConcentration=\"0.05\" boundaryCondition=\"false\" />\r\n    </listOfSpecies>\r\n    <listOfParameters>\r\n      <parameter id=\"k1\" value=\"0.9\" constant=\"true\" />\r\n      <parameter id=\"k2\" value=\"0.3\" constant=\"true\" />\r\n      <parameter id=\"k3\" value=\"0.7\" constant=\"true\" />\r\n    </listOfParameters>\r\n    <listOfReactions>\r\n      <reaction id=\"J1\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"Xo\" stoichiometry=\"1\" />\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"x\" stoichiometry=\"1\" />\r\n        </listOfProducts>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <plus />\r\n              <cn> 0.1 </cn>\r\n              <apply>\r\n                <divide />\r\n                <apply>\r\n                  <times />\r\n                  <ci> k1 </ci>\r\n                  <apply>\r\n                    <power />\r\n                    <ci> x </ci>\r\n                    <cn type=\"integer\"> 4 </cn>\r\n                  </apply>\r\n                </apply>\r\n                <apply>\r\n                  <plus />\r\n                  <ci> k2 </ci>\r\n                  <apply>\r\n                    <power />\r\n                    <ci> x </ci>\r\n                    <cn type=\"integer\"> 4 </cn>\r\n                  </apply>\r\n                </apply>\r\n              </apply>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n      <reaction id=\"_J1\" reversible=\"false\">\r\n        <listOfReactants>\r\n          <speciesReference species=\"x\" stoichiometry=\"1\" />\r\n        </listOfReactants>\r\n        <listOfProducts>\r\n          <speciesReference species=\"w\" stoichiometry=\"1\" />\r\n        </listOfProducts>\r\n        <kineticLaw>\r\n          <math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n            <apply>\r\n              <times />\r\n              <ci> k3 </ci>\r\n              <ci> x </ci>\r\n            </apply>\r\n          </math>\r\n        </kineticLaw>\r\n      </reaction>\r\n    </listOfReactions>\r\n  </model>\r\n</sbml>";

//call with arguments, -m"modelFilePath" -r"resultFileFolder" -t"TempFolder" -s"Suites"
int main(int argc, char* argv[])
{
    cout << "RoadRunner C API Test" << endl;

// 	string sbmlFilepath = "C:\\vs\\src\\roadrunner\\models\\feedback.xml";
	//string sbmlFilepath = "C:\\vs\\src\\roadrunner\\models\\bistable.xml";
	//string sbmlFilepath = "C:\\vs\\src\\roadrunner\\models\\simple.xml";
	//string sbmlFilepath = "C:\\vs\\src\\roadrunner\\models\\squareWaveModel.xml";
	//string sbmlFilepath = "C:\\Users\\Wilbert\\Desktop\\BIOMD0000000009.xml";
	//string sbmlFilepath = "C:\\Users\\Wilbert\\Desktop\\BIOMD0000000203.xml";

	RRHandle _handle = createRRInstance();
	loadSBML(_handle, feedback_sbml);


	RRStringArray *strArray;
	char *settingName, *settingDesc, *settingHint;
	char *_intgList;
	int numIntgs;
	int settingType;



	// Grab info on all implemented integrators.
	numIntgs = getNumberOfIntegrators(_handle);
	printf("Number of integrators:\t %d\n", numIntgs);
	_intgList = stringArrayToString(getListOfIntegrators(_handle));

	// Probe default (CVODE) integrator
	printf("%s \n", getIntegratorDescription(_handle));
	printf("%s \n", getIntegratorHint(_handle));
	printf("%d \n", getNumberOfIntegratorParameters(_handle));

	strArray = getListOfIntegratorParameterNames(_handle);
	for (int i = 0; i < strArray->Count; ++i)
	{
		settingName = strArray->String[i];
		settingDesc = getIntegratorParameterDescription(_handle, settingName);
		settingHint = getIntegratorParameterHint(_handle, settingName);
		settingType = getIntegratorParameterType(_handle, settingName);

		printf("%s\n", settingName);
		printf("Type: %d\nDescription: %s\nHint: %s\n\n", settingType, settingDesc, settingHint);
	}

	// Add Gillespie Integrator to the mix and then grab updated info on all implemented integrators.
	setIntegrator(_handle, "gillespie");
	numIntgs = getNumberOfIntegrators(_handle);
	printf("Number of integrators:\t %d\n", numIntgs);
	_intgList = stringArrayToString(getListOfIntegrators(_handle));

	// Probe Gillespie integrator
	printf("%s \n", getIntegratorDescription(_handle));
	printf("%s \n", getIntegratorHint(_handle));
	printf("%d \n", getNumberOfIntegratorParameters(_handle));

	strArray = getListOfIntegratorParameterNames(_handle);
	for (int i = 0; i < strArray->Count; ++i)
	{
		settingName = strArray->String[i];
		settingDesc = getIntegratorParameterDescription(_handle, settingName);
		settingHint = getIntegratorParameterHint(_handle, settingName);
		settingType = getIntegratorParameterType(_handle, settingName);

		printf("%s\n", settingName);
		printf("Type: %d\nDescription: %s\nHint: %s\n\n", settingType, settingDesc, settingHint);
	}

	// Simulate
	RRCDataPtr result;
	result = simulateEx(_handle, 0, 10, 100);
	printf(rrCDataToString(result));
	freeRRCData(result);
	
	char a;
	cin >> a;
    return 0;
}



