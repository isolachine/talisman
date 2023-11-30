
class Point {
public:
  int x;
  int y;
};

int main () {
  int key;

  Point p;
  Point* q;

  p.x = key;
  p.y = key;

  q->x = key;
  q->y = key;


  if (p.x == 4) // Vulnerable + Not reported due to whitelist
    ;
  if (p.y == 4) // Vulnerable + Reported
    ;

  if (q->x == 4) // Vulnerable + Reported
    ;
  if (q->y == 4) // Vulnerable + Not reported due to whitelist
    ;
}
