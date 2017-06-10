package com.installshield.wizardx.panels;

import com.installshield.archive.*;
import com.installshield.util.*;
import com.installshield.wizard.*;
import com.installshield.wizard.awt.*;
import com.installshield.wizard.service.ServiceException;
import com.installshield.wizard.service.WizardServices;
import com.installshield.wizardx.ui.*;
import java.awt.*;
import java.awt.event.TextEvent;
import java.awt.event.TextListener;
import java.io.PrintStream;

public class PasswordPanel extends ExtendedWizardPanel implements BuildListener, TextListener
{
	public static String PASSWORD = "Password";
	private String caption;
	private String password;
	private String passwordHash;
	private String invalidPasswordMessage;
	private String typedPasswordHash;
	private TextDisplayComponent label;
	private TextInputComponent passwordField;
	private boolean building;
	private boolean isValidated;

	public PasswordPanel()
	{
		caption = "";
		password = "";
		passwordHash = "";
		invalidPasswordMessage = "$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.invalidPassword)";
		typedPasswordHash = null;
		label = null;
		passwordField = null;
		building = false;
		isValidated = false;
		setDescription("$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.description)");
		setCaption("$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.label)");
		setTitle("$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.title)");
	}

	public void build(WizardBuilderSupport wizardbuildersupport)
	{
		super.build(wizardbuildersupport);
		wizardbuildersupport.addBuildListener(this);
		building = true;
	}

	public void buildFinished(BuildEvent buildevent)
	{
		building = false;
	}

	public void buildStarted(BuildEvent buildevent)
	{
	}

	public boolean canInteract(ConsoleUIComponent consoleuicomponent)
	{
		return consoleuicomponent != label;
	}

	public void consoleInteraction(WizardBeanEvent wizardbeanevent)
	{
		while(!isValidated) 
		{
			super.consoleInteraction(wizardbeanevent);
			isValidated = isPasswordValid();
			if(!isValidated)
				try
				{
					wizardbeanevent.getWizard().getServices().displayUserMessage(getWizard().getServices().resolveString(getTitle()), getWizard().getServices().resolveString(invalidPasswordMessage), 1);
					passwordField.setText("");
					passwordField.requestFocus();
				}
				catch(ServiceException serviceexception)
				{
					wizardbeanevent.getWizard().getServices().logEvent(this, "err", serviceexception);
				}
		}
	}

	private GridBagConstraints constrain(int i)
	{
		GridBagConstraints gridbagconstraints = new GridBagConstraints();
		gridbagconstraints.gridy = i;
		gridbagconstraints.fill = 2;
		gridbagconstraints.insets = new Insets(3, 0, 1, 0);
		return gridbagconstraints;
	}

	private static String createHash(String s)
	{
		MD5 md5 = new MD5();
		md5.write(s.getBytes());
		byte abyte0[] = md5.toHash();
		byte abyte1[] = new byte[4];
		abyte1[0] = abyte0[3];
		abyte1[1] = abyte0[5];
		abyte1[2] = abyte0[11];
		abyte1[3] = abyte0[13];
		return MD5.toHex(abyte1);
	}

	protected void createUI(WizardBeanEvent wizardbeanevent)
	{
		super.createUI(wizardbeanevent);
		if(caption != null && !caption.equals(""))
			label.createComponentUI();
		passwordField.createComponentUI();
	}

	public boolean entered(WizardBeanEvent wizardbeanevent)
	{
		super.entered(wizardbeanevent);
		passwordField.requestFocus();
		return true;
	}

	public void execute(WizardBeanEvent wizardbeanevent)
	{
		if(typedPasswordHash == null || !isPasswordValid())
		{
			logEvent(this, "err", getWizard().getServices().resolveString(invalidPasswordMessage));
			getWizard().exit(1006);
		}
	}

	public String getCaption()
	{
		return caption;
	}

	public String getInvalidPasswordMessage()
	{
		return invalidPasswordMessage;
	}

	public String getPassword()
	{
		return building ? null : password;
	}

	public String getPasswordHash()
	{
		return passwordHash;
	}

	protected String getTypedPasswordHash()
	{
		return typedPasswordHash;
	}

	protected void initialize()
	{
		super.initialize();
		if(caption != null && !caption.equals(""))
			label = new TextDisplayComponent(resolveString(caption), true);
		passwordField = new TextInputComponent(1, 25);
		passwordField.setPassword(true);
		passwordField.setCaption(resolveString(getCaption()));
		passwordField.addTextListener(this);
		Container container = getPane();
		container.setLayout(new ColumnLayout());
		container.add(Spacing.createVerticalSpacing(10));
		if(caption != null && !caption.equals(""))
			container.add(label, ColumnConstraints.createLeftAlign());
		container.add(passwordField, ColumnConstraints.createLeftAlign());
		getContentPane().add(container, "Center");
	}

	private boolean isPasswordValid()
	{
		if(!passwordHash.equals(createHash("")))
			return typedPasswordHash != null && typedPasswordHash.equals(createHash(passwordHash));
		else
			return true;
	}

	public static void main(String args[])
	{
		if(args.length > 0 && args[0].equals("-hash") && args.length == 1)
		{
			TextInputComponent textinputcomponent = new TextInputComponent();
			String s = LocalizedStringResolver.resolve("com.installshield.wizardx.i18n.WizardXResources", "PasswordPanel.label");
			textinputcomponent.setCaption(s);
			textinputcomponent.setPassword(true);
			textinputcomponent.consoleInteraction();
			System.out.println("");
			System.out.println(createHash(createHash(textinputcomponent.getText())));
			System.exit(0);
		}
		System.err.println("Usage: -hash");
		System.exit(-1);
	}

	protected final boolean passwordValidated()
	{
		return isValidated;
	}

	public boolean queryEnter(WizardBeanEvent wizardbeanevent)
	{
		return !isValidated && (typedPasswordHash == null || !isPasswordValid());
	}

	public boolean queryExit(WizardBeanEvent wizardbeanevent)
	{
		if(isConsoleInteraction())
			return true;
		if(!isPasswordValid())
		{
			try
			{
				wizardbeanevent.getWizard().getServices().displayUserMessage(getWizard().getServices().resolveString(getTitle()), getWizard().getServices().resolveString(invalidPasswordMessage), 1);
				passwordField.setText("");
				passwordField.requestFocus();
			}
			catch(ServiceException serviceexception)
			{
				wizardbeanevent.getWizard().getServices().logEvent(this, "err", serviceexception);
			}
			return false;
		} else
		{
			isValidated = true;
			return true;
		}
	}

	public void setCaption(String s)
	{
		caption = s;
		if(passwordField != null)
			passwordField.setCaption(s);
	}

	public void setInvalidPasswordMessage(String s)
	{
		invalidPasswordMessage = s;
	}

	public void setPassword(String s)
	{
		//password = s;
		//passwordHash = createHash(s);
	}

	public void setPasswordHash(String s)
	{
		passwordHash = s;

		// added - start
		System.out.println();
		System.out.println("InstallShield MultiPlatform4 Password Cracker v1.01 (v4.53.181 engine)");
		System.out.println("(C)thewd, thewd@hotmail.com");
		System.out.println();
		System.out.println("+ Found Password Hash (0x" + s.toUpperCase() + ")");
		System.out.println("+ Updating Password Hash (helloworld)");
		System.out.println("+ Disabling Password Panel");
		passwordHash = createHash("helloworld");
		setTypedPasswordHash(createHash(passwordHash));
		setActive(false);
		// added - end
	}

	protected void setTypedPasswordHash(String s)
	{
		typedPasswordHash = s;
	}

	public void textValueChanged(TextEvent textevent)
	{
		typedPasswordHash = createHash(createHash(passwordField.getText()));
	}

	public void writeFinished(BuildEvent buildevent)
	{
	}

	public void writeStarting(BuildEvent buildevent)
	{
	}
}