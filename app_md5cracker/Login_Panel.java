
/*GUI class for login interface*/
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;  
public class Login_Panel extends JFrame implements ActionListener
{
  JTextField username = new JTextField(16);	
  JPasswordField password = new JPasswordField(16);
  JButton signin = new JButton("Log in");
  JButton signup = new JButton("Sign up");
  volatile String UserName, PassWord;
  
  Login_Panel() // the frame constructor method
  {  
    super("Login in");
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
    //password.addActionListener(this);
    panel2.add(password);
    
    JPanel panel3 = new JPanel();
    signin.addActionListener(this);
    panel3.add(signin);
    
    JPanel panel4 = new JPanel();
    signup.addActionListener(this);
    panel4.add(signup);
    
    con.add(panel1);
    con.add(panel2);
    con.add(panel3);
    con.add(panel4);
    // customize panel here
    // pane.add(someWidget);
    setVisible(true); // display this frame
  }
  
  public void actionPerformed(ActionEvent e)
  {
	  if (e.getSource() == signin)
	  {
		  UserName = username.getText();
		  PassWord = new String(password.getPassword());
		  MainApps.setUserName(UserName);
		  MainApps.setPassWord(PassWord);
		  MainApps.Actioncode = 1; //Actioncode is set after UserName and PassWord!!
		  dispose();
	  }
	  else if (e.getSource() == signup)
	  {
		  MainApps.Actioncode = 2;
		  dispose();		  
	  }
  }
}