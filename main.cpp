/*
 *  Mrtp: A simple raytracing tool.
 *  Copyright (C) 2017  Mikolaj Feliks <mikolaj.feliks@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <iostream>
using namespace std;

#include <sstream>
#include <string>
#include <ctime>

#include "world.hpp"
#include "parser.hpp"


/* Default settings. */
#define DEFAULT_OUTPUT   "output.png"
#define DEFAULT_WIDTH     640
#define DEFAULT_HEIGHT    480
#define DEFAULT_FOV        70.0f
#define DEFAULT_DISTANCE   60.0f
#define DEFAULT_SHADOW      0.25f
#define DEFAULT_MODEL     LIGHT_MODEL_QUADRATIC

/* Program limits. */
#define MIN_FOV        50.0f
#define MAX_FOV       170.0f
#define MIN_WIDTH     320
#define MAX_WIDTH    4096
#define MIN_HEIGHT    240
#define MAX_HEIGHT   3072

/* Exit codes. */
#define EXIT_OK    0
#define EXIT_FAIL  1


void HelpScreen (string program) {
    cout << "Usage: " << program << " [OPTION]... FILE" << endl;
    cout << "Options:\n"
            "    -h, --help\n"
            "      Print this help screen.\n\n"
            /*
            "    -q, --quiet\n"
            "      Supress all output, except errors.\n\n"
            "    -v, --version\n"
            "      Version of the program.\n\n"
            */
            "    -r, --resolution\n"
            "      Resolution of the rendered image, for\n"
            "      example 640x480 (default), 1024x768, etc.\n\n"
            "    -o, --output\n"
            "      Filename for the rendered image, in PNG\n"
            "      format (default is \"output.png\").\n\n"
            "    -f, --fov\n"
            "      Field of vision, in degrees (default is 70).\n\n"
            "    -d, --distance\n"
            "      Distance to quench light (default is 60).\n\n"
            "    -m, --model\n"
            "      Light quenching model (none, linear, quadratic,\n"
            "      default is quadratic).\n\n"
            "    -s, --shadow\n"
            "      Shadow factor (default is 0.25).\n\n"
            "Example:\n";
    cout << "    " << program << " -r 1024x768 -o test.png test.txt" << endl;
}

int main (int argc, char **argv) {
    unsigned int i, timeStart, timeStop;
    string  text, next, foo, bar;
    stringstream convert ("test");
    size_t pos;
    bool   resok;

    /* 
     * Modifiable parameters.
     */
    bool    quiet       = false;
    string  input       = "";
    string  output      = DEFAULT_OUTPUT;
    double  fov         = DEFAULT_FOV;
    double  distance    = DEFAULT_DISTANCE;
    double  shadow      = DEFAULT_SHADOW;
    char    model       = DEFAULT_MODEL;
    unsigned int width  = DEFAULT_WIDTH;
    unsigned int height = DEFAULT_HEIGHT;


    if (argc < 2) {
        HelpScreen (argv[0]);
        return EXIT_FAIL;
    }
    for (i = 1; i < argc; i++) {
        text = argv[i]; 
        if ((text == "-h") || (text == "--help")) {
            HelpScreen (argv[0]);
            return EXIT_OK;
        }
        else if ((text == "-q") || (text == "--quiet")) {
            quiet = true;
        }
        else if ((text == "-v") || (text == "--version")) {
            cout << "Version: XXX" << endl;
            return EXIT_OK;
        }
        else if ((text == "-r") || (text == "--resolution")) {
            if (i == (argc - 1)) {
                cout << "Resolution not given." << endl;
                return EXIT_FAIL;
            }
            next  = argv[++i];
            pos   = next.find ('x');
            resok = false;
            if (pos != string::npos) {
                resok = true;
            }
            else {
                pos = next.find ('X');
                if (pos != string::npos) {
                    resok = true;
                }
            }
            if (resok) {
                foo = next.substr (0, pos);
                bar = next.substr (pos + 1, next.length () - pos - 1);
                convert.str (foo);
                convert >> width;
                if (!convert) {
                    resok = false;
                }
                else {
                    convert.clear ();
                    convert.str (bar);
                    convert >> height;
                    if (!convert) {
                        resok = false;
                    }
                    convert.clear ();
                }
                if ((width < MIN_WIDTH) || (width > MAX_WIDTH) ||
                    (height < MIN_HEIGHT) || (height > MAX_HEIGHT)) {
                    resok = false;
                }
            }
            if (!resok) {
                cout << "Invalid resolution." << endl;
                return EXIT_FAIL;
            }
        }
        else if ((text == "-f") || (text == "--fov")) {
            if (i == (argc - 1)) {
                cout << "Field of vision not given." << endl;
                return EXIT_FAIL;
            }
            next = argv[++i];
            convert.str (next);
            convert >> fov;
            if (!convert) {
                cout << "Unable to convert fov to double." << endl;
                return EXIT_FAIL;
            }
            convert.clear ();
            if ((fov < MIN_FOV) || (fov > MAX_FOV)) {
                cout << "Invalid fov." << endl;
                return EXIT_FAIL;
            }
        }
        else if ((text == "-o") || (text == "--output")) {
            if (i == (argc - 1)) {
                cout << "Output file not given." << endl;
                return EXIT_FAIL;
            }
            output = argv[++i];
            /* Check for a valid filename. */
        }
        else if ((text == "-m") || (text == "--model")) {
            if (i == (argc - 1)) {
                cout << "Light quenching model not given." << endl;
                return EXIT_FAIL;
            }
            next = argv[++i];
            if (next == "none") {
                model = LIGHT_MODEL_NONE;
            }
            else if (next == "linear") {
                model = LIGHT_MODEL_LINEAR;
            }
            else if (next == "quadratic") {
                model = LIGHT_MODEL_QUADRATIC;
            }
            else {
                cout << "Unsupported light model." << endl;
                return EXIT_FAIL;
            }
        }
        else if ((text == "-d") || (text == "--distance")) {
            if (i == (argc - 1)) {
                cout << "Distance to quench light not given." << endl;
                return EXIT_FAIL;
            }
            next = argv[++i];
            convert.str (next);
            convert >> distance;
            if (!convert) {
                cout << "Unable to convert distance to double." << endl;
                return EXIT_FAIL;
            }
        }
        else if ((text == "-s") || (text == "--shadow")) {
            if (i == (argc - 1)) {
                cout << "Shadow factor not given." << endl;
                return EXIT_FAIL;
            }
            next = argv[++i];
            convert.str (next);
            convert >> shadow;
            if (!convert) {
                cout << "Unable to convert shadow to double." << endl;
                return EXIT_FAIL;
            }
        }
        else {
            if (text.at (0) == '-') {
                cout << "Undefined option: \"" << text << "\"" << endl;
                return EXIT_FAIL;
            }
            input = text;
            /* Check for a valid filename. */
        }
    }

    if (input == "") {
        cout << "No input file." << endl;
        return EXIT_FAIL;
    }

    Parser parser (input);
    parser.Parse ();
    if (parser.GetStatus () != STATUS_OK) {
        cout << "Error parsing file: \"" << input 
            << "\"" << endl;
        return EXIT_FAIL;
    }

    World world (&parser, width, height, fov, 
        distance, shadow, model);
    world.Initialize ();

    cout << "Rendering..." << endl;
    timeStart = clock ();
    world.Render ();
    timeStop = clock ();

    cout << "OK. Elapsed time: " << 
        ((timeStop - timeStart) / double (CLOCKS_PER_SEC)) << 
        " sec" << endl;
    world.WritePNG (output);

    return EXIT_OK;
}
