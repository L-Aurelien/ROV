import processing.serial.*;
import net.java.games.input.*;
import org.gamecontrolplus.*;
import org.gamecontrolplus.gui.*;

ControlIO control;
Configuration config;
ControlDevice gpad;

float x;
float y;
float z;
String sb;
int moteur1;
int moteur2;
int moteur3;
Serial myPort;

String[] list;
float[] list2={0,0,0,0,0};

void setup()
{
 size(400,400,P3D);
 myPort = new Serial(this, Serial.list()[0],9600);
 noFill();
 stroke(255);
 textSize(16);
 surface.setTitle("ROV");
 // Initialise the ControlIO
 control = ControlIO.getInstance(this);
 // Find a gamepad that matches the configuration file. To match with any 
 // connected device remove the call to filter.
 gpad = control.filter(GCP.GAMEPAD).getMatchedDevice("xbox360");
 if (gpad == null) {
   println("No suitable device configured");
   System.exit(-1); // End the program NOW!
  }
}

public void getUserInput() {
  x= gpad.getSlider("AxeX").getValue();
  y= gpad.getSlider("AxeY").getValue();
  z= -gpad.getSlider("RotationY").getValue();
}

void draw()
{
  getUserInput();
  moteur1=int(map((x-y),-2,2,-255,255));
  moteur2=int(map((-y-x),-2,2,-255,255));
  moteur3=int(map(z,-1,1,-255,255));
  String azer=moteur1+":"+moteur2+":"+moteur3;
  myPort.write(azer);
  if (myPort.available()>0) 
  {
    sb=myPort.readStringUntil('\n');
    if (sb!=null) 
    {
      list= sb.split(":");
      if (list.length>=3) 
      {
        list2= float(list);
      }
    }
  }    
  
  background(0);
  translate(width/2,height/2);
  pushMatrix();
  rotateX(radians(list2[0]));
  rotateY(radians(list2[2]));
  rotateZ(radians(-list2[1]));

  fill(255, 0, 0, 200);
  box(10, 10, 200);

  fill(0, 0, 255, 200);
  pushMatrix();
  translate(0, 0, -120);
  rotateX(PI/2);
  drawCylinder(0, 20, 20, 8);
  popMatrix();

  fill(0, 255, 0, 200);
  beginShape(TRIANGLES);
  vertex(-100,  2, 30); vertex(0,  2, -80); vertex(100,  2, 30);  
  vertex(-100, -2, 30); vertex(0, -2, -80); vertex(100, -2, 30);  
  vertex(-2, 0, 98); vertex(-2, -30, 98); vertex(-2, 0, 70);  
  vertex( 2, 0, 98); vertex( 2, -30, 98); vertex( 2, 0, 70);  
  endShape();
  beginShape(QUADS);
  vertex(-100, 2, 30); vertex(-100, -2, 30); vertex(  0, -2, -80); vertex(  0, 2, -80);
  vertex( 100, 2, 30); vertex( 100, -2, 30); vertex(  0, -2, -80); vertex(  0, 2, -80);
  vertex(-100, 2, 30); vertex(-100, -2, 30); vertex(100, -2,  30); vertex(100, 2,  30);
  vertex(-2,   0, 98); vertex(2,   0, 98); vertex(2, -30, 98); vertex(-2, -30, 98);
  vertex(-2,   0, 98); vertex(2,   0, 98); vertex(2,   0, 70); vertex(-2,   0, 70);
  vertex(-2, -30, 98); vertex(2, -30, 98); vertex(2,   0, 70); vertex(-2,   0, 70);
  endShape();
  popMatrix();
  
  text("Température : "+list2[3]+" °C",-width/2,16-height/2);
  text("Pression : "+list2[4]+" mBar",-width/2,32-height/2);
  text("moteur 1 : "+moteur1,-width/2,48-height/2);
  text("moteur 2 : "+moteur2,-width/2,64-height/2);
  text("moteur 3 : "+moteur3,-width/2,80-height/2);
  text(azer,-width/2,96-height/2);
}

void drawCylinder(float topRadius, float bottomRadius, float tall, int sides) {
    float angle = 0;
    float angleIncrement = TWO_PI / sides;
    beginShape(QUAD_STRIP);
    for (int i = 0; i < sides + 1; ++i) {
        vertex(topRadius*cos(angle), 0, topRadius*sin(angle));
        vertex(bottomRadius*cos(angle), tall, bottomRadius*sin(angle));
        angle += angleIncrement;
    }
    endShape();

    if (topRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);

        vertex(0, 0, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(topRadius * cos(angle), 0, topRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }

    if (bottomRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);
        vertex(0, tall, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(bottomRadius * cos(angle), tall, bottomRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }
}
