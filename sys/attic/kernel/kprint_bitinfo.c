/********************************************************************************************************************/
//
//void kprint_bitinfo(bitinfo_t *info, uint32_t bits)
//{
//    uint32_t u = bits;
//    int i = 0;
//
//    while (info[i].bit != 0)
//    {
//        if ((u & info[i].bit) == info[i].bit)
//        {
//            putstr(info[i].name);
//
//            u &= ~info[i].bit;
//            if (u)
//                putstr(" ");
//        }
//
//        ++i;
//    }
//}
//
/********************************************************************************************************************/