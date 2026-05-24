using System;
using System.Collections.Generic;
using System.Text;

namespace NewsletterSender
{
    public class WOUser
    {
        public string email = "lukasccb@gmail.com";
        public string first = "Lukas";
        public string last = "CCB";

        public WOUser()
        {
        }

        public WOUser(string in_email)
        {
            email = in_email;
        }
    }
}
