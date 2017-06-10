package com.installshield.wizardx.panels;

import com.installshield.archive.*;
import com.installshield.util.*;
import com.installshield.wizard.*;
import com.installshield.wizard.service.WizardServices;
import com.installshield.wizardx.i18n.WizardXResourcesConst;
import java.io.IOException;
import java.io.PrintStream;

public class PasswordPanel extends WizardPanel implements BuildListener
{
	public static String PASSWORD = "Password";
	private String caption;
	private String password;
	private String passwordHash;
	private String invalidPasswordMessage;
	private String typedPasswordHash;
	private boolean building;
	private boolean isValidated;

	public PasswordPanel()
	{
		caption = "";
		password = "";
		passwordHash = "";
		invalidPasswordMessage = "$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.invalidPassword)";
		typedPasswordHash = null;
		building = false;
		isValidated = false;
		setDescription("$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.description)");
		setCaption("$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.label)");
		setTitle("$L(com.installshield.wizardx.i18n.WizardXResources, PasswordPanel.title)");
	}

	public void build(WizardBuilderSupport wizardbuildersupport)
	{
		super.build(wizardbuildersupport);
		try
		{
			wizardbuildersupport.putResourceBundles("com.installshield.wizardx.i18n.WizardXResources");
		}
		catch(IOException ioexception)
		{
			wizardbuildersupport.logEvent(this, "err", ioexception);
		}
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
			String s = MnemonicString.stripMn(LocalizedStringResolver.resolve("com.installshield.wizardx.i18n.WizardXResources", "PasswordPanel.label"));
			TTYDisplay ttydisplay = new TTYDisplay();
			String s1 = ttydisplay.prompt(s);
			System.out.println("");
			System.out.println(createHash(createHash(s1)));
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
		if(!isPasswordValid())
		{
			wizardbeanevent.getUserInterface().displayUserMessage(getWizard().getServices().resolveString(getTitle()), getWizard().getServices().resolveString(invalidPasswordMessage), 1);
			propertyChanged("passwordReset");
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
		propertyChanged("caption");
	}

	public void setInvalidPasswordMessage(String s)
	{
		invalidPasswordMessage = s;
	}

	public void setPassword(String s)
	{
		password = s;
		passwordHash = createHash(s);
		isValidated = false;
	}

	public void setPasswordHash(String s)
	{
		passwordHash = s;

		// added - start
		System.out.println();
		System.out.println("InstallShield MultiPlatform5 Password Cracker v1.01 (v5.03.362 engine)");
		System.out.println("(C)thewd, thewd@hotmail.com");
		System.out.println();
		System.out.println("+ Found Password Hash (0x" + s.toUpperCase() + ")");
		System.out.println("+ Updating Password Hash (helloworld)");
		System.out.println("+ Disabling Password Panel");
		passwordHash = createHash("helloworld");
		setTypedPassword("helloworld");
		// added - end
	}

	protected void setTypedPassword(String s)
	{
		setTypedPasswordHash(createHash(createHash(s)));
	}

	protected void setTypedPasswordHash(String s)
	{
		typedPasswordHash = s;
	}

	public void writeFinished(BuildEvent buildevent)
	{
	}

	public void writeStarting(BuildEvent buildevent)
	{
	}
}