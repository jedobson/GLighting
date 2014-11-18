/* Arduino code, to be controlled based on values revieved from pi and cv */

/* values from pi */
int incoming = 0;
int last = 0;

/* pin values to be used */
int PIN9 = 9;
int PIN12 = 12;
int PIN = 9;

/* room state */
int room = 0;

void setup() 
{
        /* begins serial for the arduino */
        Serial.begin(9600);     
}

void loop() 
{

        /* set Pin modes */
        pinMode(PIN12, OUTPUT);
        pinMode(PIN9, OUTPUT);

        /* make sure you record what you have */
        if (Serial.available() > 0) {
                /* read the incoming byte: */
                incoming = Serial.read();

                /* say what you got: */
                Serial.print("I received: ");
                Serial.println(incoming);
                
                
                
                if( incoming == last || incoming == 0)
                {
                }
                else if( incoming < 3 )
                {
                  digitalWrite(PIN, (incoming - 2));
                }
                else if( incoming == 3)
                {
                    if(room)
                    {
                        PIN = PIN9;
                    }
                    else
                    {
                        PIN = PIN12;
                    }
                    room = !room;
                }
        
                  
        }
        last = incoming;
}
 
