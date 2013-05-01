
/*GUI class for sign up interface*/
import java.awt.*;
import java.awt.event.*;
import javax.swing.*; 
public class Signup_Panel extends JFrame implements ActionListener
{
	JTextField username = new JTextField(16);	
	JPasswordField password1 = new JPasswordField(16);
	JPasswordField password2 = new JPasswordField(16);
	JButton signup = new JButton("Sign up");
	String UserName, PassWord1, PassWord2;
	
	Signup_Panel()
	{
		super("Sign up");
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
	    double width = screenSize.getWidth();
	    double height = screenSize.getHeight();
	    setBounds((int)(width*3/8),(int)(height*3/8),(int)(width/4),(int)(height/4));
	    setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    
	    Container con = this.getContentPane(); // inherit main frame
	    setLayout(new BoxLayout(con, BoxLayout.Y_AXIS));
	    JPanel panel1 = new JPanel();
	    JLabel label1 = new JLabel("Username",JLabel.TRAILING);
	    panel1.add(label1);
	    //username.addActionListener(this);
	    panel1.add(username);
	    
	    JPanel panel2 = new JPanel();
	    JLabel label2 = new JLabel("Password",JLabel.TRAILING);
	    panel2.add(label2);
	    //password1.addActionListener(this);
	    panel2.add(password1);

	    JPanel panel3 = new JPanel();
	    JLabel label3 = new JLabel("Password",JLabel.TRAILING);
	    panel3.add(label3);
	    //password2.addActionListener(this);
	    panel3.add(password2);
	
	    JPanel panel4 = new JPanel();
	    signup.addActionListener(this);
	    panel4.add(signup);	
	    
	    con.add(panel1);
	    con.add(panel2);	
	    con.add(panel3);
	    con.add(panel4);
	    
	    setVisible(true);
	}
	public void actionPerformed(ActionEvent e)
	{
		  if (e.getSource() == signup)
		  {
			  UserName = username.getText();
			  PassWord1 = new String(password1.getPassword());
			  PassWord2 = new String(password2.getPassword());
			  if (PassWord1.equals(PassWord2))
			  {
				  MainApps.setUserName(UserName);
				  MainApps.setPassWord(PassWord1);
				  MainApps.Actioncode = 3;//Actioncode is set after UserName and PassWord!!
				  dispose();
			  }		  
			  else{
				  JOptionPane.showMessageDialog(null, "Please confirm your message carefully!");
				  //To do sth here
			  }
		  }
	}
}
