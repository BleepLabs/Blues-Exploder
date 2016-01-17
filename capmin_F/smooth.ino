int digitalSmooth(int rawIn, int *sensSmoothArray){ 
  int j, k, temp, top, bottom;
  long total;
  static int i;

  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  // throw out two top and two bottom samples

  bottom = 1; 
  top = (filterSamples - 1);  
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j]; 
    k++; 
  }

  return total >>2;    // div by 8 number of samples
}

