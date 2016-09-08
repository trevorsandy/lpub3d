    QList<QString> ldrData;
    ldrData << "0 FILE printbanner.ldr";
    ldrData << "0 Name: printbanner.ldr";
    ldrData << "0 Author: Trevor SANDY";
    ldrData << "0 Unofficial Model";
    ldrData << "0 !LEOCAD MODEL NAME Printbanner";
    ldrData << "0 !LEOCAD MODEL AUTHOR Trevor SANDY";
    ldrData << "0 !LEOCAD MODEL DESCRIPTION Graphic displayed during pdf printing";
    ldrData << "0 !LEOCAD MODEL BACKGROUND IMAGE NAME " + imageFile;
    ldrData << "1 71 0 0 0 1 0 0 0 1 0 0 0 1 3020.dat";
    ldrData << "1 71 30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 -30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 -30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 -30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    ldrData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    ldrData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    ldrData << "1 2 -30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    ldrData << "1 71 0 -24 10 1 0 0 0 1 0 0 0 1 3937.dat";
    ldrData << "1 72 0 -8 -10 1 0 0 0 1 0 0 0 1 3023.dat";
    ldrData << "1 72 0 -8 -10 -1 0 0 0 1 0 0 0 -1 85984.dat";
    ldrData << "1 71 0 -23.272 6.254 -1 0 0 0 0.927 0.375 0 0.375 -0.927 3938.dat";
    ldrData << "1 72 0 -45.524 -2.737 -1 0 0 0 0.927 0.375 0 0.375 -0.927 4865a.dat";
    switch (type) {
      case EXPORT_PNG:
		ldrData << "1 25 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
		ldrData << "1 25 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptn.dat";
		ldrData << "1 25 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";	  
        break;
      case EXPORT_JPG:	 
	  	ldrData << "1 92 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptj.dat";
		ldrData << "1 92 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
		ldrData << "1 92 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
      case EXPORT_BMP:
	  	ldrData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
		ldrData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptm.dat";
		ldrData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        break;
      default:	  
	    ldrData << "1 216 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
		ldrData << "1 216 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
		ldrData << "1 216 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptf.dat";
      }
    ldrData << "0";
    ldrData << "0 NOFILE";
		
		
		
		
		
		
		
		
		