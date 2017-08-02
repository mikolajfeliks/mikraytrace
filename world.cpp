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
#include "world.hpp"


World::World (Parser *parser, unsigned int width,
        unsigned int height, double fov,
        double distance, double shadowfactor, 
        char lightmodel) {
    /* 
     * Initialize. 
     */
    nplanes    = 0;
    nspheres   = 0;
    ncylinders = 0;

    planes     = NULL;
    spheres    = NULL;
    cylinders  = NULL;
    buffer     = NULL;
    camera     = NULL;
    light      = NULL;

    maxdist  = distance;
    shadow   = shadowfactor;
    model    = lightmodel;

    parser_  =  parser;
    width_   =  width;
    height_  =  height;
    fov_     =  fov;
}

bool World::Initialize () {
    Entry   entry;
    string  label;
    unsigned int nentries;

    string  key,
        texts[MAX_COMPONENTS];
    double  reals[MAX_COMPONENTS];
    char    type;
    unsigned int i;

    /*
     * Allocate buffer.
     */
    buffer = new Buffer (width_, height_);
    buffer->Allocate ();

    /*
     * Allocate camera, light, actors, etc.
     */
    do {
        nentries = parser_->PopEntry (&entry);
        entry.GetLabel (&label);
        i = MAX_LINES;

        /* DEBUG
        entry.Print (); */

        /*
         * Add a camera.
         */
        if (label == "camera") {
            Vector position, target;
            double roll;

            while (entry.GetData (&key, &type, reals, texts, &i)) {
                if (key == "position") {
                    position.Set (reals);
                }
                else if (key == "target") {
                    target.Set (reals);
                }
                else {  /* if (key == "roll") */
                    roll = reals[0];
                }
            }
            camera = new Camera (&position, &target, width_, 
                height_, fov_, roll);
        }

        /*
         * Add a light.
         */
        else if (label == "light") {
            Vector position;

            entry.GetData (&key, &type, reals, texts, &i);
            position.Set (reals);
            light = new Light (&position);
        }

        /*
         * Add a plane.
         */
        else if (label == "plane") {
            Vector  center, normal;
            Color   ca, cb;
            double  scale;

            while (entry.GetData (&key, &type, reals, texts, &i)) {
                if (key == "center") {
                    center.Set (reals);
                }
                else if (key == "normal") {
                    normal.Set (reals);
                }
                else if (key == "scale") {
                    scale = reals[0];
                }
                else if (key == "cola") {
                    /*  ca.Set ((float) reals[0], (float) reals[1], 
                        (float) reals[2]);  */
                    ca.Set (1.0f, 0.0f, 1.0f);
                }
                else {  /* if (key == "colb") */
                    /* cb.Set ((float) reals[0], (float) reals[1], 
                        (float) reals[2]); */
                    ca.Set (0.0f, 0.0f, 1.0f);
                }
            }
            AddPlane (&center, &normal, &ca, &cb, scale);
        }

        /*
         * Add a sphere.
         */
        else if (label == "sphere") {
            Vector  position;
            double  radius;
            Color   color;

            while (entry.GetData (&key, &type, reals, texts, &i)) {
                if (key == "position") {
                    position.Set (reals);
                }
                else if (key == "radius") {
                    radius = reals[0];
                }
                else {  /* if (key == "color") */
                    /* color.Set ((float) reals[0], (float) reals[1], 
                        (float) reals[2]); */
                    color.Set (0.0f, 1.0f, 0.0f);
                }
            }
            AddSphere (&position, radius, &color);
        }

        /*
         * Add a cylinder.
         */
        else if (label == "cylinder") {
            Vector  axisa, axisb;
            double  radius;
            Color   color;

            while (entry.GetData (&key, &type, reals, texts, &i)) {
                if (key == "a") {
                    axisa.Set (reals);
                }
                else if (key == "b") {
                    axisb.Set (reals);
                }
                else if (key == "radius") {
                    radius = reals[0];
                }
                else {  /* if (key == "color") */
                    /* color.Set ((float) reals[0], (float) reals[1], 
                        (float) reals[2]); */
                    color.Set (1.0f, 1.0f, 1.0f);
                }
            }
            AddCylinder (&axisa, &axisb, radius, &color);
        }
    } while (nentries > 0);

    return true;
}

World::~World () {
    if (camera != NULL) {
        delete camera;
    }
    if (light  != NULL) {
        delete light;
    }
    if (buffer != NULL) {
        delete buffer;
    }

    /* Clear all planes. */
    do {
        PopPlane ();
    } while (nplanes > 0);

    /* Clear all spheres. */
    do {
        PopSphere ();
    } while (nspheres > 0);

    /* Clear all cylinders. */
    do {
        PopCylinder ();
    } while (ncylinders > 0);
}

unsigned int World::PopPlane () {
    Plane *prev, *next, *last;

    if (nplanes > 0) {
        last = planes;
        prev = NULL;
        while ((next = last->GetNext ()) != NULL) {
            prev = last;
            last = next;
        }
        if (prev != NULL)
            prev->SetNext (NULL);
        delete last;
        nplanes--;
    }
    /* Returns zero if there are no planes left. */ 
    return nplanes;
}

unsigned int World::PopSphere () {
    Sphere *prev, *next, *last;

    if (nspheres > 0) {
        last = spheres;
        prev = NULL;
        while ((next = last->GetNext ()) != NULL) {
            prev = last;
            last = next;
        }
        if (prev != NULL)
            prev->SetNext (NULL);
        delete last;
        nspheres--;
    }
    /* Returns zero if there are no spheres left. */ 
    return nspheres;
}

unsigned int World::PopCylinder () {
    Cylinder *prev, *next, *last;

    if (ncylinders > 0) {
        last = cylinders;
        prev = NULL;
        while ((next = last->GetNext ()) != NULL) {
            prev = last;
            last = next;
        }
        if (prev != NULL)
            prev->SetNext (NULL);
        delete last;
        ncylinders--;
    }
    /* Returns zero if there are no cylinders left. */ 
    return ncylinders;
}

unsigned int World::AddPlane (Vector *center, Vector *normal,
        Color *colora, Color *colorb, double texscale) {
    Plane *next, *last, *plane;

    plane = new Plane (center, normal, colora, 
        colorb, texscale);
    if (nplanes < 1) {
        planes = plane;
    }
    else {
        next = planes;
        do {
            last = next;
            next = last->GetNext ();
        } while (next != NULL);
        last->SetNext (plane);
    }
    return (++nplanes);
}

unsigned int World::AddSphere (Vector *center, double radius,
        Color *color) {
    Sphere *next, *last, *sphere;

    sphere = new Sphere (center, radius, color);
    if (nspheres < 1) {
        spheres = sphere;
    }
    else {
        next = spheres;
        do {
            last = next;
            next = last->GetNext ();
        } while (next != NULL);
        last->SetNext (sphere);
    }
    return (++nspheres);
}

unsigned int World::AddCylinder (Vector *A, Vector *B, 
        double radius, Color *color) {
    Cylinder *next, *last, *cylinder;

    cylinder = new Cylinder (A, B, radius, color);
    if (ncylinders < 1) {
        cylinders = cylinder;
    }
    else {
        next = cylinders;
        do {
            last = next;
            next = last->GetNext ();
        } while (next != NULL);
        last->SetNext (cylinder);
    }
    return (++ncylinders);
}

void World::TraceRay (Vector *origin, Vector *direction,
        Color *color) {
    Plane    *plane, *hitplane;
    Sphere   *sphere, *hitsphere;
    Cylinder *cylinder, *hitcylinder;
    double    dist, currd, dot, raylen, fade;
    char      hit;
    Vector    inter, tl, normal;
    Color     objcol;
    bool      isshadow;

    /*
     * Initialize.
     */
    color->Zero ();
    currd  = maxdist;
    hit    = HIT_NULL;
    /*
     * Search for planes.
     */
    plane    = planes;
    hitplane = NULL;
    while (plane != NULL) {
        dist = plane->Solve (origin, direction, 0., maxdist);
        if ((dist > 0.) && (dist < currd)) {
            currd    = dist;
            hitplane = plane;
            hit      = HIT_PLANE;
        }
        plane = plane->GetNext ();
    }

    /*
     * Search for spheres.
     */
    sphere    = spheres;
    hitsphere = NULL;
    while (sphere != NULL) {
        dist  = sphere->Solve (origin, direction, 0., maxdist);
        if ((dist > 0.) && (dist < currd)) {
            currd     = dist;
            hitsphere = sphere;
            hit       = HIT_SPHERE;
        }
        sphere = sphere->GetNext ();
    }

    /*
     * Search for cylinders. 
     */
    cylinder    = cylinders;
    hitcylinder = NULL;
    while (cylinder != NULL) {
        dist  = cylinder->Solve (origin, direction, 0., maxdist);
        if ((dist > 0.) && (dist < currd)) {
            currd       = dist;
            hitcylinder = cylinder;
            hit         = HIT_CYLINDER;
        }
        cylinder = cylinder->GetNext ();
    }

    if (hit != HIT_NULL) {
        /*
         * Found intersection of the current ray 
         *   and an object.
         */
        inter = ((*direction) * currd) + (*origin);

        switch (hit) {
            case HIT_PLANE:
                hitplane->GetNormal (&normal);
                hitplane->DetermineColor (&inter, &objcol);
                break;
            case HIT_SPHERE:
                hitsphere->GetNormal (&inter, &normal);
                hitsphere->DetermineColor (&objcol);
                break;
            case HIT_CYLINDER:
                hitcylinder->GetNormal (&inter, &normal);
                hitcylinder->DetermineColor (&objcol);
                break;
        }
        /*
         * Find a vector between the intersection
         *   and light.
         *
         */
        light->GetToLight (&inter, &tl);
        raylen = tl.Len ();
        tl.Normalize_InPlace ();
        dot = normal * tl;

        /*
         * Planes cannot cast shadows so check only for
         * spheres and cylinder.
         *
         */
        isshadow = false;
        sphere   = spheres;
        while (sphere != NULL) {
            if (sphere != hitsphere) {
                dist = sphere->Solve (&inter, &tl, 0., raylen);
                if (dist > 0.) {
                    isshadow = true;
                    break;
                }
            }
            sphere = sphere->GetNext ();
        }
        if (!isshadow) {
            cylinder   = cylinders;
            while (cylinder != NULL) {
                if (cylinder != hitcylinder) {
                    dist = cylinder->Solve (&inter, &tl, 0., raylen);
                    if (dist > 0.) {
                        isshadow = true;
                        break;
                    }
                }
                cylinder = cylinder->GetNext ();
            }
        }

        if (isshadow) {
            dot *= shadow;
        }
        /*
         * Decrease light intensity for objects further
         * away from the light.
         *
         */
        if (model == LIGHT_MODEL_LINEAR) {
            fade = 1.0 - (raylen / maxdist);
        }
        else if (model == LIGHT_MODEL_QUADRATIC) {
            fade = 1.0 - sqr (raylen / maxdist);
        }
        else {  /* if (model == LIGHT_MODEL_NONE) */
            fade = 1.0;
        }
        dot *= fade;
        objcol.Scale_InPlace (dot);
        /*
         * Put a pixel in the frame buffer.
         */
        objcol.CopyTo (color); 
    }
}

void World::Render () {
    Color *bp;
    Vector vw, vh, vo, eye,
        horiz, verti, origin, direction;
    unsigned int width, height, i, j;
    /*
     * Initialize.
     */
    camera->CalculateVectors (&vw, &vh, &vo);
    camera->GetDimensions (&width, &height);
    camera->GetEye (&eye);

    bp = buffer->GetPointer ();
    /*
     * Trace a ray for every pixel.
     */
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            horiz     = vw * (double) i;
            verti     = vh * (double) j;
            origin    = vo + horiz + verti;
            direction = origin - eye;
            direction.Normalize_InPlace ();

            TraceRay (&origin, &direction, bp++);
        }
    }
}

void World::WritePNG (string filename) {
    /*
    Color blue (0., 0., 1.);
    buffer->Text ("BUFFER TEST.", 0, 0, &blue);
    */
    buffer->WriteToPNG (filename);
}
