import javax.swing.JOptionPane;


public class MainApps {
	volatile static int Actioncode = 0;
	static String UserName, PassWord;
	static String filename = "Userdata.txt";
	public static void main(String args[]){
			
		new Login_Panel();
		int flag = 0;
		while (flag!=1)
		{
			if (Actioncode == 1) //sign in
			{
				flag = DataBase.Search(UserName, PassWord); 
				if (flag == 1)
				{
					JOptionPane.showMessageDialog(null, "Sign in successfully!");
				}
				else {
					JOptionPane.showMessageDialog(null, "Usernmae or password is incorrect!");
					Actioncode = 0;  //block the while loop
					new Login_Panel();
				}
			}
			else if (Actioncode == 2)// sign up
			{
				Actioncode = 0;  //block the while loop
				new Signup_Panel();
				
			}
			else if (Actioncode == 3)// add newly created user data into database
			{
				flag = DataBase.Insert(UserName, PassWord);
				if (flag == 1)
				{
					JOptionPane.showMessageDialog(null, "Registeration is completed!");
				}
				else{ 
					JOptionPane.showMessageDialog(null, "Username is already used by someone else! Please choose another username!");
					Actioncode = 0;  //block the while loop
					new Signup_Panel();
				}
			}
		}
	}
	
	static public void setUserName(String s)
	{
		UserName = new String(s);
	}
	
	static public void setPassWord(String s)
	{
		PassWord = new String(s);
	}
}
