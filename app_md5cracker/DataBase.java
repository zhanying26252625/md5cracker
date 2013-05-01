import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/*Data base class manipulating user data*/

public class DataBase {

	static public int Insert(String key, String value)
	{
		FileReader file = null;
		FileWriter fw = null;
		int flag = 0;
		
		try {
		    file = new FileReader(MainApps.filename);
		    BufferedReader reader = new BufferedReader(file);
		    String line = "";
		    char[] s;
		    String temp;
		    int i;
		    
		    while ((line = reader.readLine()) != null) {
		    	temp = "";
		    	s = line.toCharArray();
		    	i = 0;
		    	while (s[i] != ' ')
		    	{
		    		temp += s[i];
		    		i++;
		    	}
		    	if (key.equals(new String(temp)))
		    	{
		    		flag = 1;
		    		break;
		    	}
		    }
		    reader.close();
		  } catch (Exception e) {
		      throw new RuntimeException(e);
		  } finally {
		    if (file != null) {
		      try {
		        file.close();
		      } catch (IOException e) {
		        // Ignore issues during closing 
		      }
		   }
		  }

		if (flag == 0) 
			{
				try{
					fw = new FileWriter(MainApps.filename,true);
					BufferedWriter writer = new BufferedWriter(fw);
					
					value = DataBase.generateMD5(value);
					writer.write(key+' '+value+'\n');
					writer.close();
				}
				catch (Exception e) {
				      throw new RuntimeException(e);
				  } finally {
					    if (fw != null) {
						      try {
						        fw.close();
						      } catch (IOException e) {
						        // Ignore issues during closing 
						      }
						   }
				  }
				return 1;
			}
		else return 0;
	}
	
	static public int Search(String key, String value)
	{
		FileReader file = null;
		int flag = 0;
		
		try {
		    file = new FileReader(MainApps.filename);
		    BufferedReader reader = new BufferedReader(file);
		    String line = "";
		    char[] s;
		    String temp;
		    int i;
		    
		    while ((line = reader.readLine()) != null) {
		    	temp = "";
		    	s = line.toCharArray();
		    	i = 0;
		    	while (s[i] != ' ')
		    	{
		    		temp += s[i];
		    		i++;
		    	}
		    	if (key.equals(new String(temp)))
		    	{
		    		i = i+1;
		    		temp = "";
		    		while (i<s.length)
		    		{
		    			temp += s[i];
		    			i++;
		    		}
		    		
		    		value = DataBase.generateMD5(value);
		    		if (value.equals(new String(temp)))
		    		{
		    			flag = 1;
		    		}
		    		break;
		    	}
		    }
		    reader.close();
		  } catch (Exception e) {
		      throw new RuntimeException(e);
		  } finally {
		    if (file != null) {
		      try {
		        file.close();
		      } catch (IOException e) {
		        // Ignore issues during closing 
		      }
		   }
		  }
		return flag;
	}
	
	static public String generateMD5(String s) throws NoSuchAlgorithmException{
		
		MessageDigest m = MessageDigest.getInstance("MD5");
		m.reset();
		m.update(s.getBytes());
		byte[] digest = m.digest();
		BigInteger bigInt = new BigInteger(1,digest);
		String hashtext = bigInt.toString(16);
		// Now we need to zero pad it if you actually want the full 32 chars.
		while(hashtext.length() < 32 ){
		  hashtext = "0"+hashtext;		
		}
		return hashtext;
	}
}
