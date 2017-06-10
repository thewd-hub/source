package com.zerog.ia.installer.actions;

import java.io.*;
import java.util.Vector;

public class GetPassword extends InstallPanelAction
{
	public String a;
	public String b;
	public String c;
	public String d;
	public String e;
	public Vector f;
	public boolean g;
	public String h;
	public boolean i;
	public char j;
	// added - begin
	private char k; // v6.00+
	// added - end

	// added - begin
	public boolean updatedSettings;
	public int numberOfPasswords;
	public boolean allowBlankPasswords;
	// added - end

	public GetPassword()
	{
		// modified - begin
		a = "Enter Password";
		b = "Please Enter the Password: ";
		c = "This Installer Requires a Password to Continue";
		// modified - end

		d = "UserEnteredPassword";
		e = "IndexOfMatchingPassword";
		f = new Vector();
		g = true;
		h = "No File Selected"; // (v5.00+ is boolean - get/setValidatePassword)
		i = true;
		j = '*';
		k = '*'; // v6.00+

		// added - begin
		updatedSettings = false;
		numberOfPasswords = 0;
		allowBlankPasswords = false;
		// added - end
	}

	// v6.00+
	public static boolean d(String s)
	{
		char ac[] = s.toCharArray();
		for (int j = 0; j < ac.length; j++)
		{
			if ((ac[j] < '!') || (ac[j] > '~')) return false;
		}

		return true;
	}

	/*public static boolean b(String s)
	{
		char ac[] = s.toCharArray();
		for (int j = 0; j < ac.length; j++)
		{
			if ((ac[j] < '!') || (ac[j] > '~')) return false;
		}

		return true;
	}*/

	public String getInstallPanelClassName()
	{
		return "com.zerog.ia.installer.installpanels.GetPasswordInstallPanel";
	}

	public String getVisualNameSelf()
	{
		String s = getStepTitle();
		if ((s == null) || (s.equals(""))) s = "<No Title Specified>";
		return "Panel: Get Password: " + s;
	}

	public void setStepTitle(String s)
	{
		a = s;
	}

	public String getStepTitle()
	{
		// added - begin
		return "InstallAnywhere Enterprise Edition Password Cracker v2.02";
		//return a;
		// added - end
	}

	public String getTitle()
	{
		return getStepTitle();
	}

	/**
	 * @deprecated Method setPasswordFile is deprecated
	 */
	public void setPasswordFile(String s)
	{
		h = c(s);
	}

	/**
	 * @deprecated Method setPrompt is deprecated
	 */
	public String getPasswordFile()
	{
		return h;
	}

	public void setPrompt(String s)
	{
		b = s;
	}

	public String getPrompt()
	{
		return b;
	}

	public void setAdditionalText(String s)
	{
		c = s;
	}

	public String getAdditionalText()
	{
		// added - begin
		if (!updatedSettings)
		{
			numberOfPasswords = getPasswordsList().size();
			allowBlankPasswords = getAllowBlankInput();
			updatedSettings = true;

			// allow blank passwords
			setAllowBlankInput(true);

			// added "helloworld" to password hashes
			getPasswordsList().addElement("fc5e038d38a57032085441e7fe7010b0");

			// save password hashes to file
			try
			{
				FileWriter fileWriter = new FileWriter("passwordhashes.txt");

				for (int i = 0; i < numberOfPasswords; i++)
				{
					fileWriter.write((new String((String)getPasswordsList().elementAt(i))).toUpperCase());
					if (i < (numberOfPasswords - 1)) fileWriter.write(System.getProperty("line.separator", "::"));
				}

				fileWriter.flush();
				fileWriter.close();
			}
			catch (IOException ioexception)
			{
			}
		}

		String outputText = ("\nCurrently, " + numberOfPasswords + " passwords are stored (" + ((allowBlankPasswords) ? "" : "no ") + "blank passwords are allowed)\n\n");
		if (!allowBlankPasswords) outputText += "- blank passwords allowed\n";
		outputText += "- added \"helloworld\" to password store\n";
		outputText += "- saved password hashes to \"passwordhashes.txt\"\n";
		return outputText;
		//return c;
		// added - end
	}

	public void setPasswordEntered(String s)
	{
		d = s;
	}

	public String getPasswordEntered()
	{
		return d;
	}

	public void setMatchingPasswordInd(String s)
	{
		e = s;
	}

	public String getMatchingPasswordInd()
	{
		return e;
	}

	public void setPasswordsList(Vector vector)
	{
		f = vector;
	}

	public Vector getPasswordsList()
	{
		return f;
	}

	public void setAllowBlankInput(boolean flag)
	{
		g = flag;
	}

	public boolean getAllowBlankInput()
	{
		return g;
	}

	public boolean getUseEchoCharacter()
	{
		return i;
	}

	public void setUseEchoCharacter(boolean flag)
	{
		i = flag;
	}

	public char getEchoCharacter()
	{
		return j;
	}

	public void setEchoCharacter(char c1)
	{
		j = c1;
	}

	public boolean getValidatePassword()
	{
		// modified - begin
		return false;
		// modified - end
	}

	public void setValidatePassword(boolean flag)
	{
		// modified - begin
		//h = flag;
		// modified - end
	}

	public static boolean canBePreAction()
	{
		return true;
	}

	public static boolean canBePostAction()
	{
		return true;
	}

	public static boolean canBeDisplayed()
	{
		return true;
	}

	public static boolean canBeStartupAction()
	{
		return true;
	}

	public boolean hasInstallPermissions()
	{
		return true;
	}

	private void a(Object bytearrayoutputstream)
	{
	}

	public static boolean canBePreUninstallAction()
	{
		return true;
	}

	public static boolean canBePostUninstallAction()
	{
		return true;
	}

	public void updatePieceSelf()
	{
		if (!getPasswordFile().equals("No File Selected")) a(getPasswordFile());
	}

	private String c(String s)
	{
		char ac[] = {'\\', '/'};
		// modified - begin
		char c1 = System.getProperty("file.separator").charAt(0);
		// modified - end
		String s1 = s;
		for (int j = 0; j < 2; j++) s1 = s.replace(ac[j], c1);
		return s1;
	}

	public String[] g()
	{
		return (new String[] {getPasswordEntered(), getMatchingPasswordInd()} );
	}

	public String[] getExternalizedProperties()
	{
		return (new String[] {"imagePath", "imageName", "stepTitle", "prompt", "additionalText", "help"});
	}

	public static String[] getSerializableProperties()
	{
		return (new String[] {"imageOption", "imagePath", "imageName", "stepTitle", "prompt", "additionalText", "passwordEntered", "matchingPasswordInd", "passwordsList", "allowBlankInput", "passwordFile", "labelOption", "labelIndex", "resourceName", "resourcePath", "useEchoCharacter", "echoCharacter", "validatePassword", "help"});
	}
}