
//
// This is a GUI support code to the chapters 12-16 of the book
// "Programming -- Principles and Practice Using C++" by Bjarne Stroustrup
//

#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include "chapter19_ex14_Graph.h"

//------------------------------------------------------------------------------

namespace Graph_lib {;

//------------------------------------------------------------------------------

Shape::Shape() :
    lcolor(fl_color()),      // default color for lines and characters
    ls(0),                   // default style
    fcolor(Color::invisible) // no fill
{}

//------------------------------------------------------------------------------

void Shape::add(Point p)     // protected
{
    points.push_back(p);
}

//------------------------------------------------------------------------------

void Shape::set_point(int i, Point p)        // not used; not necessary so far
{
    points[i] = p;
}

//------------------------------------------------------------------------------

void Shape::draw_lines() const
{
    if (color().visibility() && 1<points.size())    // draw sole pixel?
        for (unsigned int i=1; i<points.size(); ++i)
            fl_line(points[i-1].x,points[i-1].y,points[i].x,points[i].y);
}

//------------------------------------------------------------------------------

void Shape::draw() const
{
    Fl_Color oldc = fl_color();
    // there is no good portable way of retrieving the current style
    fl_color(lcolor.as_int());            // set color
    fl_line_style(ls.style(),ls.width()); // set style
    draw_lines();
    fl_color(oldc);      // reset color (to previous)
    fl_line_style(0);    // reset line style to default
}

//------------------------------------------------------------------------------

void Shape::move(int dx, int dy)    // move the shape +=dx and +=dy
{
    for (int i = 0; i<points.size(); ++i) {
        points[i].x+=dx;
        points[i].y+=dy;
    }
}

//------------------------------------------------------------------------------

Line::Line(Point p1, Point p2)    // construct a line from two points
{
    add(p1);    // add p1 to this shape
    add(p2);    // add p2 to this shape
}

//------------------------------------------------------------------------------

void Lines::add(Point p1, Point p2)
{
    Shape::add(p1);
    Shape::add(p2);
}

//------------------------------------------------------------------------------

// draw lines connecting pairs of points
void Lines::draw_lines() const
{
    if (color().visibility())
        for (int i=1; i<number_of_points(); i+=2)
            fl_line(point(i-1).x,point(i-1).y,point(i).x,point(i).y);
}

//------------------------------------------------------------------------------

// does two lines (p1,p2) and (p3,p4) intersect?
// if se return the distance of the intersect point as distances from p1
inline pair<double,double> line_intersect(Point p1, Point p2, Point p3, Point p4, bool& parallel)
{
    double x1 = p1.x;
    double x2 = p2.x;
    double x3 = p3.x;
    double x4 = p4.x;
    double y1 = p1.y;
    double y2 = p2.y;
    double y3 = p3.y;
    double y4 = p4.y;

    double denom = ((y4 - y3)*(x2-x1) - (x4-x3)*(y2-y1));
    if (denom == 0){
        parallel= true;
        return pair<double,double>(0,0);
    }
    parallel = false;
    return pair<double,double>( ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3))/denom,
                                ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3))/denom);
}

//------------------------------------------------------------------------------

//intersection between two line segments
//Returns true if the two segments intersect,
//in which case intersection is set to the point of intersection
bool line_segment_intersect(Point p1, Point p2, Point p3, Point p4, Point& intersection){
   bool parallel;
   pair<double,double> u = line_intersect(p1,p2,p3,p4,parallel);
   if (parallel || u.first < 0 || u.first > 1 || u.second < 0 || u.second > 1) return false;
   intersection.x = p1.x + u.first*(p2.x - p1.x);
   intersection.y = p1.y + u.first*(p2.y - p1.y);
   return true;
}

//------------------------------------------------------------------------------

void Polygon::add(Point p)
{
    int np = number_of_points();

    if (1<np) {    // check that the new line isn't parallel to the previous one
        if (p==point(np-1)) error("polygon point equal to previous point");
        bool parallel;
        line_intersect(point(np-1),p,point(np-2),point(np-1),parallel);
        if (parallel)
            error("two polygon points lie in a straight line");
    }

    for (int i = 1; i<np-1; ++i) {    // check that new segment doesn't interset and old point
        Point ignore(0,0);
        if (line_segment_intersect(point(np-1),p,point(i-1),point(i),ignore))
            error("intersect in polygon");
    }


    Closed_polyline::add(p);
}

//------------------------------------------------------------------------------

void Polygon::draw_lines() const
{
    if (number_of_points() < 3) error("less than 3 points in a Polygon");
    Closed_polyline::draw_lines();
}

//------------------------------------------------------------------------------

Regular_polygon::Regular_polygon(Point cc, int nn, int rr)
    :n(nn), rad(rr)
{
    add(cc);
    for (int i = 0; i<nn; ++i)  // add empty points
        add(Point());
    set_points(cc,nn,rr);
}

//------------------------------------------------------------------------------

void Regular_polygon::draw_lines() const
{
    if (fill_color().visibility()) {
        fl_color(fill_color().as_int());
        fl_begin_complex_polygon();
        for (int i = 1; i<number_of_points(); ++i)  // point(0) is center
            fl_vertex(point(i).x,point(i).y);
        fl_end_complex_polygon();
        fl_color(color().as_int());    // reset color
    }

    if (color().visibility()) {
        for (int i = 1; i<number_of_points()-1; ++i)
            fl_line(point(i).x,point(i).y,point(i+1).x,point(i+1).y);
        fl_line(point(number_of_points()-1).x,
            point(number_of_points()-1).y,
            point(1).x,
            point(1).y);   // close polygon
    }
}

//------------------------------------------------------------------------------

void Regular_polygon::set_points(Point xy, int n, int r)
{
    rad = r;
    double alpha = 2*pi/n;      // inner angle
    double phi = (pi+alpha)/2;  // current angle
    for (int i = 0; i<n; ++i) {
        set_point(i+1,Point(round(center().x+r*cos(phi+i*alpha)),
            round(center().y+r*sin(phi+i*alpha))));
    }
}

//------------------------------------------------------------------------------

void Open_polyline::draw_lines() const
{
    if (fill_color().visibility()) {
        fl_color(fill_color().as_int());
        fl_begin_complex_polygon();
        for(int i=0; i<number_of_points(); ++i){
            fl_vertex(point(i).x, point(i).y);
        }
        fl_end_complex_polygon();
        fl_color(color().as_int());    // reset color
    }

    if (color().visibility())
        Shape::draw_lines();
}

//------------------------------------------------------------------------------

void Closed_polyline::draw_lines() const
{
    Open_polyline::draw_lines();    // first draw the "open poly line part"
    // then draw closing line:
    if (color().visibility()) {
        fl_line(point(number_of_points()-1).x,
            point(number_of_points()-1).y,
            point(0).x,
            point(0).y);
    }
}

//------------------------------------------------------------------------------

void draw_mark(Point xy, char c)
{
    static const int dx = 4;
    static const int dy = 4;

    string m(1,c);
    fl_draw(m.c_str(),xy.x-dx,xy.y+dy);
}

//------------------------------------------------------------------------------

void Marked_polyline::draw_lines() const
{
    Open_polyline::draw_lines();
    for (int i=0; i<number_of_points(); ++i)
        draw_mark(point(i),mark[i%mark.size()]);
}

//------------------------------------------------------------------------------

void Rectangle::draw_lines() const
{
    if (fill_color().visibility()) {    // fill
        fl_color(fill_color().as_int());
        fl_rectf(point(0).x,point(0).y,w,h);
        fl_color(color().as_int());    // reset color
    }

    if (color().visibility()) {    // lines on top of fill
        fl_color(color().as_int());
        fl_rect(point(0).x,point(0).y,w,h);
    }
}

//------------------------------------------------------------------------------

Point n(const Rectangle& rect)
{
    return Point(rect.point(0).x+rect.width()/2,rect.point(0).y);
}

//------------------------------------------------------------------------------

Point s(const Rectangle& rect)
{
    return Point(rect.point(0).x+rect.width()/2,rect.point(0).y+rect.height());
}

//------------------------------------------------------------------------------

Point e(const Rectangle& rect)
{
    return Point(rect.point(0).x+rect.width(),rect.point(0).y+rect.height()/2);
}

//------------------------------------------------------------------------------

Point w(const Rectangle& rect)
{
    return Point(rect.point(0).x,rect.point(0).y+rect.height()/2);
}

//------------------------------------------------------------------------------

Point center(const Rectangle& rect)
{
    return Point(rect.point(0).x+rect.width()/2,rect.point(0).y+rect.height()/2);
}

//------------------------------------------------------------------------------

Point ne(const Rectangle& rect)
{
    return Point(rect.point(0).x+rect.width(),rect.point(0).y);
}

//------------------------------------------------------------------------------

Point se(const Rectangle& rect)
{
    return Point(rect.point(0).x+rect.width(),rect.point(0).y+rect.height());
}

//------------------------------------------------------------------------------

Point sw(const Rectangle& rect)
{
    return Point(rect.point(0).x,rect.point(0).y+rect.height());
}

//------------------------------------------------------------------------------

Point nw(const Rectangle& rect)
{
    return rect.point(0);
}

//------------------------------------------------------------------------------

Circle::Circle(Point p, int rr)    // center and radius
    :r(rr)
{
    add(Point(p.x-r,p.y-r));       // store top-left corner
}

//------------------------------------------------------------------------------

Point Circle::center() const
{
    return Point(point(0).x+r, point(0).y+r);
}

//------------------------------------------------------------------------------

void Circle::draw_lines() const
{
  	if (fill_color().visibility()) {	// fill
		fl_color(fill_color().as_int());
		fl_pie(point(0).x,point(0).y,r+r-1,r+r-1,0,360);
		fl_color(color().as_int());	// reset color
	}

	if (color().visibility()) {
		fl_color(color().as_int());
		fl_arc(point(0).x,point(0).y,r+r,r+r,0,360);
	}
}

//------------------------------------------------------------------------------

Point n(const Circle& c)
{
    return Point(c.center().x,c.center().y-c.radius());
}

//------------------------------------------------------------------------------

Point s(const Circle& c)
{
    return Point(c.center().x,c.center().y+c.radius());
}

//------------------------------------------------------------------------------

Point e(const Circle& c)
{
    return Point(c.center().x+c.radius(),c.center().y);
}

//------------------------------------------------------------------------------

Point w(const Circle& c)
{
    return Point(c.center().x-c.radius(),c.center().y);
}

//------------------------------------------------------------------------------

Point center(const Circle& c)
{
    return c.center();
}

//------------------------------------------------------------------------------

Point ne(const Circle& c)
{
    return Point(c.center().x+c.radius()/sqrt(2),c.center().y-c.radius()/sqrt(2));
}

//------------------------------------------------------------------------------

Point se(const Circle& c)
{
    return Point(c.center().x+c.radius()/sqrt(2),c.center().y+c.radius()/sqrt(2));
}

//------------------------------------------------------------------------------

Point sw(const Circle& c)
{
    return Point(c.center().x-c.radius()/sqrt(2),c.center().y+c.radius()/sqrt(2));
}

//------------------------------------------------------------------------------

Point nw(const Circle& c)
{
    return Point(c.center().x-c.radius()/sqrt(2),c.center().y-c.radius()/sqrt(2));
}

//------------------------------------------------------------------------------

void Text::draw_lines() const
{
    int ofnt = fl_font();
    int osz = fl_size();
    fl_font(fnt.as_int(),fnt_sz);
    fl_draw(lab.c_str(),point(0).x,point(0).y);
    fl_font(ofnt,osz);
}

//------------------------------------------------------------------------------

Axis::Axis(Orientation d, Point xy, int length, int n, string lab) :
    label(Point(0,0),lab)
{
    if (length<0) error("bad axis length");
    switch (d){
    case Axis::x:
    {
        Shape::add(xy); // axis line
        Shape::add(Point(xy.x+length,xy.y));

        if (0<n) {      // add notches
            int dist = length/n;
            int x = xy.x+dist;
            for (int i = 0; i<n; ++i) {
                notches.add(Point(x,xy.y),Point(x,xy.y-5));
                x += dist;
            }
        }
        // label under the line
        label.move(length/3,xy.y+20);
        break;
    }
    case Axis::y:
    {
        Shape::add(xy); // a y-axis goes up
        Shape::add(Point(xy.x,xy.y-length));

        if (0<n) {      // add notches
            int dist = length/n;
            int y = xy.y-dist;
            for (int i = 0; i<n; ++i) {
                notches.add(Point(xy.x,y),Point(xy.x+5,y));
                y -= dist;
            }
        }
        // label at top
        label.move(xy.x-10,xy.y-length-10);
        break;
    }
    case Axis::z:
        error("z axis not implemented");
    }
}

//------------------------------------------------------------------------------

void Axis::draw_lines() const
{
    Shape::draw_lines();
    notches.draw();  // the notches may have a different color from the line
    label.draw();    // the label may have a different color from the line
}

//------------------------------------------------------------------------------

void Axis::set_color(Color c)
{
    Shape::set_color(c);
    notches.set_color(c);
    label.set_color(c);
}

//------------------------------------------------------------------------------

void Axis::move(int dx, int dy)
{
    Shape::move(dx,dy);
    notches.move(dx,dy);
    label.move(dx,dy);
}

//------------------------------------------------------------------------------

Function::Function(Fct f, double r1, double r2, Point xy,
                   int count, double xscale, double yscale)
// graph f(x) for x in [r1:r2) using count line segments with (0,0) displayed at xy
// x coordinates are scaled by xscale and y coordinates scaled by yscale
{
    if (r2-r1<=0) error("bad graphing range");
    if (count <=0) error("non-positive graphing count");
    double dist = (r2-r1)/count;
    double r = r1;
    for (int i = 0; i<count; ++i) {
        add(Point(xy.x+int(r*xscale),xy.y-int(f(r)*yscale)));
        r += dist;
    }
}

//------------------------------------------------------------------------------

Flex_function::Flex_function(Fct f, double r1, double r2, Point xy,int count,
                             double xscale, double yscale, double precision)
    :Function(f,r1,r2,xy,count,xscale,yscale),
     fct(f), range1(r1), range2(r2), origin(xy),
     c(count), xsc(xscale), ysc(yscale), prec(precision)
{
    reset();
}

//------------------------------------------------------------------------------

void Flex_function::reset_range(double r1, double r2) {
    if (r2<=r1) error("bad graphing range");
    range1 = r1;
    range2 = r2;
    reset();
}

//------------------------------------------------------------------------------

void Flex_function::reset_count(int count) {
    if (count<=0) error("non-positive graphing count");
    c = count;
    reset();
}

//------------------------------------------------------------------------------

void Flex_function::reset_xscale(double xscale) {
    if (xscale==0) error("xscale must not be zero");
    xsc = xscale;
    reset();
}

//------------------------------------------------------------------------------

void Flex_function::reset_yscale(double yscale) {
    if (yscale==0) error("yscale must not be zero");
    ysc = yscale;
    reset();
}

//------------------------------------------------------------------------------

void Flex_function::reset()
{
    double dist = (range2-range1)/c;
    double r = range1;
    clear_points();
    for (int i = 0; i<c; ++i) {
        add(Point(origin.x+int(int(r*xsc)/prec)*prec,
            origin.y-int(int(fct(r)*ysc)/prec)*prec));
        r += dist;
    }
}

//------------------------------------------------------------------------------

bool can_open(const string& s)
// check if a file named s exists and can be opened for reading
{
    ifstream ff(s.c_str());
    return bool(ff);
}

//------------------------------------------------------------------------------

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

Suffix::Encoding get_encoding(const string& s)
{
    struct SuffixMap
    {
        const char*      extension;
        Suffix::Encoding suffix;
    };

    static SuffixMap smap[] = {
        {".jpg",  Suffix::jpg},
        {".jpeg", Suffix::jpg},
        {".gif",  Suffix::gif},
    };

    for (int i = 0, n = ARRAY_SIZE(smap); i < n; i++)
    {
        int len = strlen(smap[i].extension);

        if (s.length() >= len && s.substr(s.length()-len, len) == smap[i].extension)
            return smap[i].suffix;
    }

    return Suffix::none;
}

//------------------------------------------------------------------------------

// somewhat over-elaborate constructor
// because errors related to image files can be such a pain to debug
Image::Image(Point xy, string s, Suffix::Encoding e)
    :w(0), h(0), fn(xy,"")
{
    add(xy);

    if (!can_open(s)) {    // can we open s?
        fn.set_label("cannot open \""+s+'\"');
        p = new Bad_image(30,20);    // the "error image"
        return;
    }

    if (e == Suffix::none) e = get_encoding(s);

    switch(e) {        // check if it is a known encoding
    case Suffix::jpg:
        p = new Fl_JPEG_Image(s.c_str());
        break;
    case Suffix::gif:
        p = new Fl_GIF_Image(s.c_str());
        break;
    default:    // Unsupported image encoding
        fn.set_label("unsupported file type \""+s+'\"');
        p = new Bad_image(30,20);    // the "error image"
    }
}

//------------------------------------------------------------------------------

void Image::draw_lines() const
{
    if (fn.label()!="") fn.draw_lines();

    if (w&&h)
        p->draw(point(0).x,point(0).y,w,h,cx,cy);
    else
        p->draw(point(0).x,point(0).y);
}

//------------------------------------------------------------------------------

Wumpus_map::Wumpus_map(Point cc, int rr, const vector<Wumpus::Room>& rooms)
    :labels(new Vector_ref<Text>),
    rad(rr),
    avatar(Point(0,0),"pics_and_txt/hunter.jpg")
{
    add(cc);
    for (int i = 0; i<20; ++i)
        add(Point());
    set_points();
    set_labels(rooms);
    tunnels.set_style(Line_style(Line_style::solid,2));
}

//------------------------------------------------------------------------------

void Wumpus_map::draw_lines() const
{
    for (int i = 0; i<labels->size(); ++i)
        if ((*labels)[i].color().visibility()) {
            (*labels)[i].draw();
            circles[i].draw();
        }
    tunnels.draw();
    avatar.draw();
}

//------------------------------------------------------------------------------

void Wumpus_map::show(int idx)
{
    (*labels)[idx].set_color(Color::visible);
}

//------------------------------------------------------------------------------

void Wumpus_map::set_avatar(int idx)
{
    Point new_loc = circles[idx].center();
    int dx = new_loc.x-20 - avatar.point(0).x;
    int dy = new_loc.y-20 - avatar.point(0).y;
    avatar.move(dx,dy);
}

//------------------------------------------------------------------------------

void Wumpus_map::hide_all()
{
    for (int i = 0; i<labels->size(); ++i)
        (*labels)[i].set_color(Color::invisible);
    tunnels.clear_points();
}

//------------------------------------------------------------------------------

void Wumpus_map::reset_labels(const vector<Wumpus::Room>& rooms)
{
    delete labels;
    labels = new Vector_ref<Text>;
    set_labels(rooms);
}

//------------------------------------------------------------------------------

void Wumpus_map::set_points()
{
    double dphi = 2*pi/5;  // angular increment

    // innermost pentagon
    int r1 = rad/4;     // radius for smallest pentagon
    int r2 = 0.539*rad; // radius for center points of middle pentagon
    int r3 = 2*rad/3;   // radius for corners of middle pentagon
    int r4 = 0.9*rad;   // radius for outer pentagon
    double phi1 = -pi/2;            // start at the top
    double phi2 = pi/2 - 2*dphi;    // start at top right corner
    for (int i = 0; i<5; ++i) {
        // innermost pentagon
        set_point(i+1,Point(round(center().x+r1*cos(phi1+i*dphi)),
            round(center().y+r1*sin(phi1+i*dphi))));
        // center points of middle pentagon
        set_point(6+2*i,Point(round(center().x+r2*cos(phi1+i*dphi)),
            round(center().y+r2*sin(phi1+i*dphi))));
        // corners of middle pentagon
        set_point(7+2*i,Point(round(center().x+r3*cos(phi2+i*dphi)),
            round(center().y+r3*sin(phi2+i*dphi))));
        // outer pentagon
        set_point(16+i,Point(round(center().x+r4*cos(phi2+i*dphi)),
            round(center().y+r4*sin(phi2+i*dphi))));
    }
}

//------------------------------------------------------------------------------

void Wumpus_map::set_labels(const vector<Wumpus::Room>& rooms)
{
    static const int dx = 5;
    static const int dy = 5;

    int shift = 0;  // to account for two digit labels

    for (int i = 0; i<20; ++i) {
        ostringstream oss;
        oss << rooms[i].label;
        shift = (rooms[i].label>9) * 4;
        labels->push_back(new Text(Point(point(i+1).x-dx-shift,point(i+1).y+dy),
            oss.str()));
        (*labels)[i].set_color(Color::invisible);
        if (circles.size() < 20) {
            circles.push_back(new Circle(point(i+1),15));
            circles[i].set_style(Line_style(Line_style::solid,2));
        }
    }
}

//------------------------------------------------------------------------------

} // of namespace Graph_lib

